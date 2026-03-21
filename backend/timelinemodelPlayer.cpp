// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include "timelinemodelPlayer.h"
#include <QDebug>

static const int s_numberElementInMinute = 120;

/*!
 * \brief Constructor.
 * \param parent QObject parent instance
 */
TimelineModel::TimelineModel(QObject *parent) : QAbstractListModel(parent), m_currentMinute(0)
{
    addNextMinute();
}

TimelineModel::~TimelineModel()
{
    qDeleteAll(m_timeLine);
}

/**
 * \brief Overrided QAbstractListModel method.
 * \param parent Model index.
 * \return Row count in the model.
 */
int TimelineModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_timeLine.size();
}

/**
 * \brief Overrided QAbstractListModel method.
 * \param index Model index.
 * \param role Role in the model.
 * \return Relevant role data.
 */
QVariant TimelineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case TimeStringRole:
        return m_timeLine.at(index.row())->timeString();
    case IsDisplayTimeRole:
        return m_timeLine.at(index.row())->isDisplayTime();
    default:
        return QVariant();
    }
}

/**
 * \brief Overrided QAbstractListModel method.
 * \return List of roles.
 */
QHash<int, QByteArray> TimelineModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[TimeStringRole] = "time";
    roles[IsDisplayTimeRole] = "isDisplayTime";

    return roles;
}

/**
 * \brief Adds next minute to the model.
 */
void TimelineModel::addNextMinute()
{
    beginInsertRows(QModelIndex(), m_timeLine.size(),
                    m_timeLine.size() + s_numberElementInMinute - 1);
    for (int i = 0; i < s_numberElementInMinute / 4; i++) {
        QString secondsString = QString::number(i * 2);
        QString minutesString = QString::number(m_currentMinute);
        if (secondsString.size() == 1)
            secondsString = "0" + secondsString;
        if (minutesString.size() == 1)
            minutesString = "0" + minutesString;
        QString timeString = minutesString + ":" + secondsString;
        m_timeLine.append(new TimelineBlock(timeString, true));
        m_timeLine.append(new TimelineBlock("", false));
        m_timeLine.append(new TimelineBlock("", false));
        m_timeLine.append(new TimelineBlock("", false));
    }
    m_currentMinute++;
    endInsertRows();

    QModelIndex index = createIndex(0, 0, static_cast<void *>(0));
    emit dataChanged(index, index);
}

/**
 * \brief Fills timeline model for specified minutes.
 * \param lastMinute The minute before needs to fill in.
 */
void TimelineModel::fillModel(int lastMinute)
{
    while (m_currentMinute < lastMinute + 1) {
        addNextMinute();
    }
}

/**
 * \brief Clears model.
 */
//void TimelineModel::clear()
//{
//    beginRemoveRows(QModelIndex(), 0, rowCount(QModelIndex()));
//    qDeleteAll(m_timeLine);
//    m_timeLine.clear();
//    endRemoveRows();
//    m_currentMinute = 0;
//}
void TimelineModel::clear()
{
    if (m_timeLine.isEmpty()) return;

    beginResetModel();
    qDeleteAll(m_timeLine);
    m_timeLine.clear();
    m_currentMinute = 0;
    endResetModel();
}
