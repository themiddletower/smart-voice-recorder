#include "AudioAnalyzer.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <QUrl>
#include <QVariantMap>
#include <QDebug>
#include "VADModel.h"

// FEATURES
float computeRMS(const std::vector<float>& data) {
    double sum = 0.0;
    for (float v : data) sum += v * v;
    return std::sqrt(sum / data.size());
}

float computeEnergy(const std::vector<float>& data) {
    double sum = 0.0;
    for (float v : data) sum += v * v;
    return sum / data.size();
}

float computeZCR(const std::vector<float>& data) {
    int crossings = 0;
    for (size_t i = 1; i < data.size(); i++) {
        if ((data[i] >= 0) != (data[i - 1] >= 0))
            crossings++;
    }
    return (float)crossings / data.size();
}

// MAIN
QVariantList AudioAnalyzer::analyzeFile(const QString &filePath)
{
    VADModel model;

    if (!model.load(":/backend/weights.json")) {
        qDebug() << "Failed to load weights.json";
        return {};
    }

    if (!model.loadNorm(":/backend/norm.json")) {
        qDebug() << "Failed to load norm.json";
        return {};
    }

    QVariantList result;

    QString localPath = filePath;
    if (localPath.startsWith("file://"))
        localPath = QUrl(localPath).toLocalFile();

    std::ifstream file(localPath.toStdString(), std::ios::binary);
    if (!file) return result;

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::string(header.riff, 4) != "RIFF" ||
        header.audioFormat != 1 ||
        header.bitsPerSample != 16)
        return result;

    const uint32_t sampleRate = header.sampleRate;
    const uint16_t channels = header.numChannels;
    const int frameSamples = sampleRate / 50; // 20 ms
    const double frameDurationMs = 20.0;

    std::vector<int16_t> buffer(frameSamples * channels);

    std::vector<int> labels;
    labels.reserve(10000);

    // сглаживание вероятности
    const int smoothWindow = 10;
    std::vector<float> probHistory;

    // FRAME LOOP
    while (file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int16_t))) {

        // --- mono + нормализация ---
        std::vector<float> mono(frameSamples);

        for (int j = 0; j < frameSamples; j++) {
            int32_t acc = 0;
            for (int ch = 0; ch < channels; ch++)
                acc += buffer[j * channels + ch];

            float sample = float(acc / channels) / 32768.0f; // КРИТИЧНО
            mono[j] = sample;
        }

        // --- features ---
        float rms = computeRMS(mono);
        float zcr = computeZCR(mono);
        float energy = computeEnergy(mono);

        // --- inference ---
        float prob = model.forward({rms, zcr, energy});

        qDebug() << "rms:" << rms
                 << "zcr:" << zcr
                 << "energy:" << energy
                 << "prob:" << prob;

        // --- smoothing ---
        probHistory.push_back(prob);
        if (probHistory.size() > smoothWindow)
            probHistory.erase(probHistory.begin());

        float probAvg = 0.0f;
        for (float p : probHistory) probAvg += p;
        probAvg /= probHistory.size();

        // --- decision ---
        //const float loudThreshold = 0.15f;

        //if (rms > loudThreshold) {
        //    labels.push_back(3); // громко
        //} else {
        //    labels.push_back(probAvg > 0.5f ? 1 : 2);
        //}
        labels.push_back(probAvg > 0.5f ? 1 : 2);
    }

    if (labels.empty()) return result;

    // SMOOTH LABELS
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

    // SEGMENTS
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

    QVariantMap last;
    last["t1"] = t1;
    last["t2"] = smooth.size() * frameDurationMs;
    last["type"] = currentType;
    segments.push_back(last);

    if (segments.empty()) return result;

    for (size_t i = 1; i < segments.size(); ++i) {
        double prevEnd = segments[i - 1]["t2"].toDouble();
        segments[i]["t1"] = prevEnd;
    }

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

    // PADDING
    const double speechPaddingMs = 400.0;

    for (int i = 0; i < result.size(); ++i) {
        QVariantMap seg = result[i].toMap();

        if (seg["type"].toInt() == 1 || seg["type"].toInt() == 3) {
            double t1 = seg["t1"].toDouble() - speechPaddingMs;
            double t2 = seg["t2"].toDouble() + speechPaddingMs;

            seg["t1"] = std::max(0.0, t1);
            seg["t2"] = t2;

            result[i] = seg;
        }
    }

    return result;
}
