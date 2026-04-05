#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <QVariantList>
#include <QVariantMap>
#include "AudioAnalyzer.h"
#include <QDebug>

bool applyDenoiseOnly(
    const QString &filePath,
    const QString &outputPath
    ) {
    AudioAnalyzer analyzer;

    QVariantList segments = analyzer.analyzeFile(filePath);

    std::ifstream inFile(filePath.toStdString(), std::ios::binary);
    if (!inFile) return false;

    WAVHeader header;
    inFile.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::string(header.riff, 4) != "RIFF" ||
        header.audioFormat != 1 ||
        header.bitsPerSample != 16) {
        return false;
    }

    uint16_t channels = header.numChannels;
    uint32_t sampleRate = header.sampleRate;
    uint32_t bytesPerSample = header.bitsPerSample / 8;

    std::vector<int16_t> buffer(header.dataSize / bytesPerSample);
    inFile.read(reinterpret_cast<char*>(buffer.data()), header.dataSize);
    inFile.close();

    double sumSq = 0.0;
    size_t count = 0;

    for (const QVariant &v : segments) {
        QVariantMap seg = v.toMap();

        int type = seg["type"].toInt();
        if (type != 2) continue;

        uint32_t startSample = seg["t1"].toDouble() * sampleRate / 1000;
        uint32_t endSample = seg["t2"].toDouble() * sampleRate / 1000;
        qDebug() << "[Silence] startSample:" << startSample
                 << "endSample:" << endSample;

        endSample = std::min(endSample, static_cast<uint32_t>(buffer.size() / channels));

        for (uint32_t i = startSample; i < endSample; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                double s = buffer[i * channels + ch];
                sumSq += s * s;
                count++;
            }
        }
    }

    double backgroundRMS = (count > 0) ? std::sqrt(sumSq / count) : 800.0;

    double noiseThreshold = backgroundRMS * 2;

    std::vector<int16_t> outBuffer = buffer;
    qDebug() << "noiseThreshold:"<< noiseThreshold;

    for (const QVariant &v : segments) {
        QVariantMap seg = v.toMap();
        int type = seg["type"].toInt();

        if (type != 2) continue;

        uint32_t startSample = seg["t1"].toDouble() * sampleRate / 1000;
        uint32_t endSample   = seg["t2"].toDouble() * sampleRate / 1000;

        startSample = std::min(startSample, static_cast<uint32_t>(outBuffer.size() / channels));
        endSample   = std::min(endSample,   static_cast<uint32_t>(outBuffer.size() / channels));

        for (uint32_t i = startSample; i < endSample; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                int16_t &sample = outBuffer[i * channels + ch];
                sample = static_cast<int16_t>(sample * 0.1);
            }
        }
    }

    std::ofstream outFile(outputPath.toStdString(), std::ios::binary);
    if (!outFile) return false;

    outFile.write(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
    outFile.write(reinterpret_cast<char*>(outBuffer.data()), outBuffer.size() * bytesPerSample);
    outFile.close();

    return true;
}
