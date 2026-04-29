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

    // Новый метод — просто копирует текущий временный файл в Music с новым именем
    Q_INVOKABLE bool finalizeSave(const QString &currentTempPath, const QString &newFileName);

private:
    QString makeTempWavPath() const; // временный файл в кэше
};

#endif // SILENCESERVICE_H
