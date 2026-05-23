#ifndef SILENCESERVICE_H
#define SILENCESERVICE_H

#include <QObject>
#include <QVariant>

class SilenceService : public QObject
{
    Q_OBJECT
public:
    explicit SilenceService(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap removeSilence(const QString &inputPath,
                                          const QVariantList &annotations,
                                          int silenceType);

    Q_INVOKABLE bool finalizeSave(const QString &currentTempPath, const QString &newFileName);
    Q_INVOKABLE bool saveMetadata(const QString &audioPath, const QVariantList &annotations, const QVariantList &voiceLabels);
    Q_INVOKABLE QVariantMap loadMetadata(const QString &audioPath);

    Q_INVOKABLE QString getFileHash(const QString &filePath);

    Q_INVOKABLE QString getAppDataPath() const;

    Q_INVOKABLE QVariantMap loadMetadataFromFile(const QString &jsonPath);

    Q_INVOKABLE QString getMusicPath() const;

private:
    QString makeTempWavPath() const;
};

#endif // SILENCESERVICE_H
