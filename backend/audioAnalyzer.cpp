#include "AudioAnalyzer.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <QUrl>
#include <QVariantMap>

// Структура для промежуточного анализа кадров
struct FrameInfo {
    double startTime;
    int type; // 1-речь, 2-громко, 3-тишина
};

QVariantList AudioAnalyzer::analyzeFile(const QString &filePath)
{
    QVariantList result;

    QString localPath = filePath;
    if (localPath.startsWith("file://"))
        localPath = QUrl(localPath).toLocalFile();

    std::ifstream file(localPath.toStdString(), std::ios::binary);
    if (!file) return result;

    WAVHeader header;

    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::string(header.riff, 4) != "RIFF" || header.audioFormat != 1 || header.bitsPerSample != 16)
        return result;

    const uint32_t sampleRate = header.sampleRate;
    const uint16_t channels = header.numChannels;
    const int frameSamples = sampleRate / 50; // 20 мс
    const double frameDurationMs = 20.0;

    // --- RMS ---
    auto computeRMS = [](const std::vector<int16_t>& data) -> double {
        if (data.empty()) return 0.0;
        double sum = 0.0;
        for (int16_t s : data) sum += double(s) * s;
        return std::sqrt(sum / data.size());
    };

    std::vector<double> rmsValues;
    std::vector<int16_t> buffer(frameSamples * channels);

    while (file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int16_t))) {
        std::vector<int16_t> mono(frameSamples);

        for (int j = 0; j < frameSamples; j++) {
            int32_t acc = 0;
            for (int ch = 0; ch < channels; ch++)
                acc += buffer[j * channels + ch];

            mono[j] = int16_t(acc / channels);
        }

        rmsValues.push_back(computeRMS(mono));
        qDebug() << buffer[0] << buffer[1] << buffer[2];
    }

    if (rmsValues.empty()) return result;

    // --- Пороговая классификация ---
    const double speechThreshold = 800.0;
    const double loudThreshold   = 4000.0;

    std::vector<int> labels;
    labels.reserve(rmsValues.size());

    for (double rms : rmsValues) {
        if (rms > loudThreshold) labels.push_back(2);
        else if (rms > speechThreshold) labels.push_back(1);
        else labels.push_back(3); // тишина
    }

    // --- Сглаживание (anti-flicker) ---
    const int window = 40;

    std::vector<int> smooth = labels;

    for (size_t i = 0; i < labels.size(); ++i) {
        int count[4] = {0};

        for (int k = -window; k <= window; ++k) {
            int idx = int(i) + k;
            if (idx >= 0 && idx < (int)labels.size())
                count[labels[idx]]++;
        }

        int bestType = labels[i];
        int bestCount = 0;

        for (int t = 1; t <= 3; ++t) {
            if (count[t] > bestCount) {
                bestCount = count[t];
                bestType = t;
            }
        }

        smooth[i] = bestType;
    }

    // --- Формирование сегментов (БЕЗ padding) ---
    std::vector<QVariantMap> segments;

    int currentType = smooth[0];
    double t1 = 0.0;

    for (size_t i = 1; i < smooth.size(); ++i) {
        if (smooth[i] != currentType) {
            double t2 = i * frameDurationMs;

            QVariantMap seg;
            seg["t1"] = t1;
            seg["t2"] = t2;
            seg["type"] = currentType;
            segments.push_back(seg);

            t1 = t2;
            currentType = smooth[i];
        }
    }

    // последний сегмент
    QVariantMap last;
    last["t1"] = t1;
    last["t2"] = smooth.size() * frameDurationMs;
    last["type"] = currentType;
    segments.push_back(last);

    if (segments.empty()) return result;

    // --- Гарантия непрерывности (NO GAP / NO OVERLAP) ---
    for (size_t i = 1; i < segments.size(); ++i) {
        double prevEnd = segments[i - 1]["t2"].toDouble();
        segments[i]["t1"] = prevEnd;
    }

    // --- Merge одинаковых типов ---
    QVariantMap current = segments[0];

    for (size_t i = 1; i < segments.size(); ++i) {
        QVariantMap next = segments[i];

        if (next["type"].toInt() == current["type"].toInt()) {
            current["t2"] = next["t2"];
        } else {
            result.append(current);
            current = next;
        }
    }
    result.append(current);

    const double speechPaddingMs = 400.0;

    for (int i = 0; i < result.size(); ++i) {
        QVariantMap seg = result[i].toMap();

        if (seg["type"].toInt() == 1 || seg["type"].toInt() == 2) {
            double t1 = seg["t1"].toDouble() - speechPaddingMs;
            double t2 = seg["t2"].toDouble() + speechPaddingMs;

            seg["t1"] = std::max(0.0, t1);
            seg["t2"] = t2;

            result[i] = seg;
        }
    }

    QVariantList cleanedResult;

    for (int i = 0; i < result.size(); ++i) {
        QVariantMap seg = result[i].toMap();

        // Если это тишина (тип 3)
        if (seg["type"].toInt() == 3) {
            double t1 = seg["t1"].toDouble();
            double t2 = seg["t2"].toDouble();

            // Проверяем соседа слева
            if (i > 0) {
                double prevEnd = result[i-1].toMap()["t2"].toDouble();
                if (t1 < prevEnd) t1 = prevEnd; // Подрезаем начало тишины
            }

            // Проверяем соседа справа
            if (i < result.size() - 1) {
                double nextStart = result[i+1].toMap()["t1"].toDouble();
                if (t2 > nextStart) t2 = nextStart; // Подрезаем конец тишины
            }

            // Если после подрезки от тишины что-то осталось — сохраняем
            if (t2 > t1) {
                seg["t1"] = t1;
                seg["t2"] = t2;
                cleanedResult.append(seg);
            }
            // Если t2 <= t1, значит речь полностью поглотила этот участок тишины
        } else {
            // Речь и Громко добавляем как есть (они уже расширены)
            cleanedResult.append(seg);
        }
    }
    // Заменяем result на очищенный список для дальнейшей обработки
    result = cleanedResult;

    for (int i = 1; i < result.size(); ++i) {
        QVariantMap prev = result[i - 1].toMap();
        QVariantMap curr = result[i].toMap();

        double prevEnd = prev["t2"].toDouble();

        if (curr["t1"].toDouble() < prevEnd) {
            curr["t1"] = prevEnd;
            result[i] = curr;
        }
    }

    current = result[0].toMap();
    QVariantList result_1;

    for (size_t i = 1; i < static_cast<size_t>(result.size()); ++i) {
        QVariantMap next = result[i].toMap();

        if (next["type"].toInt() == current["type"].toInt()) {
            current["t2"] = next["t2"];
        } else {
            result_1.append(current);
            current = next;
        }
    }
    result_1.append(current);

    const double minDurationMs = 500.0;
    QVariantList finalResult;

    for (int i = 0; i < result_1.size(); ++i) {
        QVariantMap seg = result_1[i].toMap();
        double duration = seg["t2"].toDouble() - seg["t1"].toDouble();
        int type = seg["type"].toInt();

        if ((type == 1 || type == 2) && duration < minDurationMs) {
            seg["type"] = 3; // Теперь мы меняем локальную копию и ПЕРЕЗАПИСЫВАЕМ её ниже
        }

        // Сразу делаем финальное слияние, чтобы тишина не дробилась
        if (!finalResult.isEmpty()) {
            QVariantMap last = finalResult.last().toMap();
            if (last["type"].toInt() == seg["type"].toInt()) {
                last["t2"] = seg["t2"];
                finalResult[finalResult.size() - 1] = last;
                continue;
            }
        }
        finalResult.append(seg);
    }

    return finalResult;
}
