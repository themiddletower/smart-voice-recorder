#ifndef AUDIOAMPLITUDEMODELPLAYER_H
#define AUDIOAMPLITUDEMODELPLAYER_H

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include "audioamplitudePlayer.h"

class AudioAmplitudeModelRedact : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AudioAmplitudeModelRedact(QObject *parent = nullptr);
    ~AudioAmplitudeModelRedact();

    enum Roles { ValueRole = Qt::UserRole + 1, IsDefaultValueRole, AnnotationTypeRole };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear();
    void setAmplitudes(const QList<qreal> &amplitudes);

    Q_INVOKABLE void applyAnnotations(const QVariantList &annotations, int measurementsPerSec = 16);

private:
    QList<AudioAmplitudeRedact *> m_audioAmplitudes;
};

#endif // AUDIOAMPLITUDEMODELPLAYER_H
