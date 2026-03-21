// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TIMELINEMODELPLAYER_H
#define TIMELINEMODELPLAYER_H

#include <QAbstractListModel>

#include "timelineblockPlayer.h"

/*!
 * \brief The TimelineModel class is the model class responsible for displaying
 * timeline for player and recorder pages.
 */
class TimelineModel : public QAbstractListModel
{
public:
    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel();

    enum Roles { TimeStringRole = Qt::UserRole + 1, IsDisplayTimeRole };

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void addNextMinute();
    Q_INVOKABLE void fillModel(int lastMinute);
    Q_INVOKABLE void clear();

private:
    QList<TimelineBlock *> m_timeLine;
    int m_currentMinute;
};

#endif // TIMELINEMODELPLAYER_H
