#include "timelinemodel.h"

static const int s_numberElementInMinute = 120;

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent), m_currentMinute(0)
{
    addNextMinute();
}

TimelineModel::~TimelineModel()
{
    qDeleteAll(m_timeLine);
}

int TimelineModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_timeLine.size();
}

QVariant TimelineModel::data(const QModelIndex &index, int role) const
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

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[TimeStringRole] = "time";
    roles[IsDisplayTimeRole] = "isDisplayTime";
    return roles;
}

void TimelineModel::addNextMinute()
{
    beginInsertRows(QModelIndex(), m_timeLine.size(),
                    m_timeLine.size() + s_numberElementInMinute - 1);
    for (int i = 0; i < s_numberElementInMinute / 4; i++) {
        QString secondsString = QString::number(i * 2);
        QString minutesString = QString::number(m_currentMinute);
        if (secondsString.size() == 1) secondsString = "0" + secondsString;
        if (minutesString.size() == 1) minutesString = "0" + minutesString;
        QString timeString = minutesString + ":" + secondsString;
        m_timeLine.append(new TimelineBlock(timeString, true));
        m_timeLine.append(new TimelineBlock("", false));
        m_timeLine.append(new TimelineBlock("", false));
        m_timeLine.append(new TimelineBlock("", false));
    }
    m_currentMinute++;
    endInsertRows();
}

void TimelineModel::fillModel(int lastMinute)
{
    while (m_currentMinute < lastMinute + 1)
        addNextMinute();
}

void TimelineModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    qDeleteAll(m_timeLine);
    m_timeLine.clear();
    endRemoveRows();
    m_currentMinute = 0;
}
