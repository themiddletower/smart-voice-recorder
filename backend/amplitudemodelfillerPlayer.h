// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AMPLITUDEMODELFILLERPLAYER_H
#define AMPLITUDEMODELFILLERPLAYER_H

#include <QObject>
#include "audioamplitudePlayer.h"

// КЛАСС-ЗАГЛУШКА: Больше не используется в новой логике
class AmplitudeModelFiller : public QObject
{
    Q_OBJECT
public:
    explicit AmplitudeModelFiller(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    void fill(QList<AudioAmplitude *> *audioAmplitudes, int elementsCount) {
        Q_UNUSED(audioAmplitudes);
        Q_UNUSED(elementsCount);
    }
signals:
    void modelFilled();
};

#endif // AMPLITUDEMODELFILLERPLAYER_H
