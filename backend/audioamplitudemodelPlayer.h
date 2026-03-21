// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AUDIOAMPLITUDEMODELPLAYER_H
#define AUDIOAMPLITUDEMODELPLAYER_H

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include "audioamplitudePlayer.h"

class AudioAmplitudeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AudioAmplitudeModel(QObject *parent = nullptr);
    ~AudioAmplitudeModel();

    enum Roles { ValueRole = Qt::UserRole + 1, IsDefaultValueRole, AnnotationTypeRole };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear();
    void setAmplitudes(const QList<qreal> &amplitudes);

    // Принимает массив: [{"t1": 1000, "t2": 5000, "type": 1}, ...] (время в миллисекундах)
    Q_INVOKABLE void applyAnnotations(const QVariantList &annotations, int measurementsPerSec = 16);

private:
    QList<AudioAmplitude *> m_audioAmplitudes;
};

#endif // AUDIOAMPLITUDEMODELPLAYER_H
