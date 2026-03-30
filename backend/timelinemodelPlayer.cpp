#include "timelinemodelPlayer.h"
#include <QDebug>

static const int s_numberElementInMinute = 120;

TimelineModelRedact::TimelineModelRedact(QObject *parent)
    : QAbstractListModel(parent), m_currentMinute(0)
{
    addNextMinute();
}

TimelineModelRedact::~TimelineModelRedact()
{
    qDeleteAll(m_timeLine);
}

int TimelineModelRedact::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_timeLine.size();
}

QVariant TimelineModelRedact::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    switch (role) {
    case TimeStringRole:
        return m_timeLine.at(index.row())->timeString();
    case IsDisplayTimeRole:
        return m_timeLine.at(index.row())->isDisplayTime();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TimelineModelRedact::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[TimeStringRole] = "time";
    roles[IsDisplayTimeRole] = "isDisplayTime";
    return roles;
}

void TimelineModelRedact::addNextMinute()
{
    beginInsertRows(QModelIndex(), m_timeLine.size(),
                    m_timeLine.size() + s_numberElementInMinute - 1);
    for (int i = 0; i < s_numberElementInMinute / 4; i++) {
        QString secondsString = QString::number(i * 2);
        QString minutesString = QString::number(m_currentMinute);
        if (secondsString.size() == 1) secondsString = "0" + secondsString;
        if (minutesString.size() == 1) minutesString = "0" + minutesString;
        QString timeString = minutesString + ":" + secondsString;
        m_timeLine.append(new TimelineBlockRedact(timeString, true));
        m_timeLine.append(new TimelineBlockRedact("", false));
        m_timeLine.append(new TimelineBlockRedact("", false));
        m_timeLine.append(new TimelineBlockRedact("", false));
    }
    m_currentMinute++;
    endInsertRows();

    QModelIndex idx = createIndex(0, 0, static_cast<void *>(0));
    emit dataChanged(idx, idx);
}

void TimelineModelRedact::fillModel(int lastMinute)
{
    while (m_currentMinute < lastMinute + 1) {
        addNextMinute();
    }
}

void TimelineModelRedact::clear()
{
    if (m_timeLine.isEmpty()) return;

    beginResetModel();
    qDeleteAll(m_timeLine);
    m_timeLine.clear();
    m_currentMinute = 0;
    endResetModel();
}
