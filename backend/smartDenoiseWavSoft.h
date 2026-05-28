#ifndef SMARTDENOISEWAVSOFT_H
#define SMARTDENOISEWAVSOFT_H

#include <QString>
#include <QVector>
#include <cstdint>
#include <QVariantList>
#include <QVariantMap>

#pragma pack(push, 1)
struct WAVHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
};
#pragma pack(pop)

struct Segment {
    uint32_t startMs;
    uint32_t endMs;
    int type;
};

bool applyDenoiseOnly(
    const QString &filePath,
    const QString &outputPath
    );

QVector<Segment> convertToSegments(const QVariantList &list);

#endif // SMARTDENOISEWAVSOFT_H
