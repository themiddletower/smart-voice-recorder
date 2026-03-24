// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#include "silenceremover_qt.h"

#include <QVariantMap>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QtMath>
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QEventLoop>
#include <QDataStream>

static bool intervalLess(const SilenceRemoverQt::Interval &a,
                         const SilenceRemoverQt::Interval &b)
{
    return a.t1 < b.t1;
}

QList<SilenceRemoverQt::Interval>
SilenceRemoverQt::collectAndMergeSilences(const QVariantList &annotations, int silenceType)
{
    QList<Interval> silences;

    for (const QVariant &v : annotations) {
        const QVariantMap m = v.toMap();
        if (m.value("type").toInt() != silenceType) continue;

        Interval in{ m.value("t1").toLongLong(), m.value("t2").toLongLong() };
        if (in.t2 > in.t1) silences.append(in);
    }

    std::sort(silences.begin(), silences.end(), intervalLess);

    QList<Interval> merged;
    for (const auto &s : silences) {
        if (merged.isEmpty() || s.t1 > merged.last().t2) merged.append(s);
        else merged.last().t2 = qMax(merged.last().t2, s.t2);
    }
    return merged;
}

// Теперь нам не нужно знать общую длительность заранее. Просто берем до бесконечности.
QList<SilenceRemoverQt::Interval>
SilenceRemoverQt::buildKeepIntervals(const QList<Interval> &silences)
{
    QList<Interval> keeps;
    qint64 cur = 0;
    for (const auto &s : silences) {
        if (s.t1 > cur) keeps.append({cur, s.t1});
        cur = qMax(cur, s.t2);
    }
    // Используем максимально возможное значение времени, чтобы забрать всё до конца файла
    keeps.append({cur, 0x7FFFFFFFFFFFFFFF});
    return keeps;
}

qint64 SilenceRemoverQt::removedBefore(const QList<Interval> &silences, qint64 t)
{
    qint64 removed = 0;
    for (const auto &s : silences) {
        if (t >= s.t2) removed += (s.t2 - s.t1);
        else if (t <= s.t1) break;
        else { removed += (t - s.t1); break; }
    }
    return removed;
}

QVariantList SilenceRemoverQt::recalcAnnotations(const QVariantList &annotations,
                                                 const QList<Interval> &silences,
                                                 int silenceType)
{
    QVariantList out;
    for (const QVariant &v : annotations) {
        const QVariantMap m = v.toMap();
        const int type = m.value("type").toInt();
        if (type == silenceType) continue;

        const qint64 t1 = m.value("t1").toLongLong();
        const qint64 t2 = m.value("t2").toLongLong();

        const qint64 nt1 = t1 - removedBefore(silences, t1);
        const qint64 nt2 = t2 - removedBefore(silences, t2);

        QVariantMap nm;
        nm["t1"] = nt1;
        nm["t2"] = nt2;
        nm["type"] = type;
        out.append(nm);
    }
    return out;
}

void SilenceRemoverQt::writeWavHeader(QDataStream &out, quint32 dataSize, int sampleRate, int channels, int sampleSize)
{
    out.writeRawData("RIFF", 4);
    out << quint32(dataSize + 36); // File size - 8
    out.writeRawData("WAVE", 4);
    out.writeRawData("fmt ", 4);
    out << quint32(16);            // Subchunk1Size (16 for PCM)
    out << quint16(1);             // AudioFormat (1 = PCM)
    out << quint16(channels);
    out << quint32(sampleRate);
    out << quint32(sampleRate * channels * (sampleSize / 8)); // ByteRate
    out << quint16(channels * (sampleSize / 8));              // BlockAlign
    out << quint16(sampleSize);                               // BitsPerSample
    out.writeRawData("data", 4);
    out << quint32(dataSize);
}

bool SilenceRemoverQt::cutAndWriteWav(const QString &inputPath,
                                      const QList<Interval> &keeps,
                                      const QString &outputWavPath,
                                      QString *error)
{
    QAudioDecoder decoder;
    decoder.setSourceFilename(inputPath);

    // Настраиваем нужный формат выхода для стандартного WAV
    QAudioFormat desiredFormat;
    desiredFormat.setChannelCount(1); // моно
    desiredFormat.setCodec("audio/pcm");
    desiredFormat.setSampleType(QAudioFormat::SignedInt);
    desiredFormat.setSampleRate(44100);
    desiredFormat.setSampleSize(16);
    decoder.setAudioFormat(desiredFormat);

    QFile outFile(outputWavPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        if (error) *error = "Cannot open output file";
        return false;
    }

    // Оставляем место под WAV заголовок (44 байта)
    outFile.seek(44);
    quint32 totalDataSize = 0;

    QEventLoop loop;
    bool success = true;

    // Читаем аудио буферы
    QObject::connect(&decoder, &QAudioDecoder::bufferReady, [&]() {
        QAudioBuffer buffer = decoder.read();

        qint64 bufStartMs = buffer.startTime() / 1000;
        qint64 bufEndMs = bufStartMs + (buffer.duration() / 1000);

        for (const auto &k : keeps) {
            // Если этот буфер полностью за пределами интервала keep — пропускаем
            if (bufStartMs >= k.t2 || bufEndMs <= k.t1) continue;

            // Вычисляем, какая часть буфера нам нужна (с точностью до кадра)
            qint64 overlapStartMs = qMax(bufStartMs, k.t1);
            qint64 overlapEndMs = qMin(bufEndMs, k.t2);

            int sampleRate = buffer.format().sampleRate();
            int bytesPerFrame = buffer.format().bytesPerFrame();

            qint64 startFrame = (overlapStartMs - bufStartMs) * sampleRate / 1000;
            qint64 endFrame = (overlapEndMs - bufStartMs) * sampleRate / 1000;

            startFrame = qBound(0LL, startFrame, (qint64)buffer.frameCount());
            endFrame = qBound(0LL, endFrame, (qint64)buffer.frameCount());

            qint64 framesToWrite = endFrame - startFrame;
            if (framesToWrite > 0) {
                const char* ptr = (const char*)buffer.constData() + (startFrame * bytesPerFrame);
                qint64 bytesToWrite = framesToWrite * bytesPerFrame;

                outFile.write(ptr, bytesToWrite);
                totalDataSize += bytesToWrite;
            }
        }
    });

    QObject::connect(&decoder, static_cast<void(QAudioDecoder::*)(QAudioDecoder::Error)>(&QAudioDecoder::error), [&](QAudioDecoder::Error err) {
        if (error) *error = "Decoder error: " + decoder.errorString();
        success = false;
        loop.quit();
    });

    QObject::connect(&decoder, &QAudioDecoder::finished, [&]() {
        loop.quit();
    });

    // Запускаем процесс декодирования и ждем его завершения
    decoder.start();
    loop.exec();

    // Записываем правильный заголовок WAV файла в начало
    if (success) {
        outFile.seek(0);
        QDataStream outStream(&outFile);
        outStream.setByteOrder(QDataStream::LittleEndian); // WAV требует Little Endian
        writeWavHeader(outStream, totalDataSize,
                       decoder.audioFormat().sampleRate(),
                       decoder.audioFormat().channelCount(),
                       decoder.audioFormat().sampleSize());
    }

    outFile.close();

    // Если произошла ошибка, удаляем битый файл
    if (!success) QFile::remove(outputWavPath);

    return success;
}

SilenceRemoveResult SilenceRemoverQt::removeSilenceToWav(const QString &inputPath,
                                                         const QVariantList &annotations,
                                                         int silenceType,
                                                         const QString &outputWavPath)
{
    SilenceRemoveResult res;

    if (inputPath.isEmpty()) {
        res.error = "inputPath is empty";
        return res;
    }
    if (!QFile::exists(inputPath)) {
        res.error = "Input file not found: " + inputPath;
        return res;
    }

    QDir().mkpath(QFileInfo(outputWavPath).absolutePath());

    const QList<Interval> silences = collectAndMergeSilences(annotations, silenceType);
    res.newAnnotations = recalcAnnotations(annotations, silences, silenceType);

    const QList<Interval> keeps = buildKeepIntervals(silences);

    QString cutErr;
    if (!cutAndWriteWav(inputPath, keeps, outputWavPath, &cutErr)) {
        res.error = cutErr.isEmpty() ? "Failed to process audio" : cutErr;
        return res;
    }

    res.outputPath = outputWavPath;
    return res;
}
