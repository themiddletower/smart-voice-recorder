#include "timelineblockPlayer.h"

TimelineBlockRedact::TimelineBlockRedact(QString timeString, bool isDisplayTime)
{
    setTimeString(timeString);
    setIsDisplayTime(isDisplayTime);
}

bool TimelineBlockRedact::isDisplayTime() const
{
    return m_isDisplayTime;
}

void TimelineBlockRedact::setIsDisplayTime(bool newIsDisplayTime)
{
    m_isDisplayTime = newIsDisplayTime;
}

const QString &TimelineBlockRedact::timeString() const
{
    return m_timeString;
}

void TimelineBlockRedact::setTimeString(const QString &newTimeString)
{
    m_timeString = newTimeString;
}
