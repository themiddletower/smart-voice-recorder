#include <QObject>
#include <QVariant>
#include <QString>
#include <QVector>
#include <cstdint>
#include <QVariantList>

#pragma pack(push, 1)
struct WAVHeader {
    char riff[4];
    uint32_t chunkSize;
    char wave[4];
    char fmt[4];
    uint32_t subchunk1Size;
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
    QString type;
    uint32_t startMs;
    uint32_t endMs;
};

Q_DECLARE_METATYPE(Segment)

class AudioAnalyzer : public QObject {
    Q_OBJECT
public:
    explicit AudioAnalyzer(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QVariantList analyzeFile(const QString &filePath);

    static QVector<Segment> analyzeWavFileInternal(
        const QString &filePath,
        int requiredStable = 5,
        uint32_t minSegmentMs = 200
        );
};
