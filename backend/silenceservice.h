#ifndef SILENCESERVICE_H
#define SILENCESERVICE_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class SilenceService : public QObject
{
    Q_OBJECT
public:
    explicit SilenceService(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap removeSilence(const QString &inputPath,
                                          const QVariantList &annotations,
                                          int silenceType = 2);

private:
    QString makeOutputWavPath(const QString &inputPath) const;
};

#endif // SILENCESERVICE_H
