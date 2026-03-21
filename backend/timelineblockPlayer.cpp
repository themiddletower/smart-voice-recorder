// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include "timelineblockPlayer.h"

/**
 * \brief Constuctor.
 * \param timeString Time string to display.
 * \param isDisplayTime Specifies whether to display the time string.
 */
TimelineBlock::TimelineBlock(QString timeString, bool isDisplayTime)
{
    setTimeString(timeString);
    setIsDisplayTime(isDisplayTime);
}

bool TimelineBlock::isDisplayTime() const
{
    return m_isDisplayTime;
}

void TimelineBlock::setIsDisplayTime(bool newIsDisplayTime)
{
    m_isDisplayTime = newIsDisplayTime;
}

const QString &TimelineBlock::timeString() const
{
    return m_timeString;
}

void TimelineBlock::setTimeString(const QString &newTimeString)
{
    m_timeString = newTimeString;
}
