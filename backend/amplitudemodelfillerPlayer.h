#ifndef AMPLITUDEMODELFILLERPLAYER_H
#define AMPLITUDEMODELFILLERPLAYER_H

#include <QObject>
#include "audioamplitudePlayer.h"

class AmplitudeModelFillerRedact : public QObject
{
    Q_OBJECT
public:
    explicit AmplitudeModelFillerRedact(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    void fill(QList<AudioAmplitudeRedact *> *audioAmplitudes, int elementsCount) {
        Q_UNUSED(audioAmplitudes);
        Q_UNUSED(elementsCount);
    }
signals:
    void modelFilled();
};

#endif // AMPLITUDEMODELFILLERPLAYER_H
