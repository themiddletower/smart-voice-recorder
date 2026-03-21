// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TIMELINEBLOCKPLAYER_H
#define TIMELINEBLOCKPLAYER_H

#include <QString>

/*!
 * \brief The TimelineBlick class describes the timeline block data structure.
 */
class TimelineBlock
{
public:
    TimelineBlock(QString timeString, bool isDisplayTime);

    bool isDisplayTime() const;
    void setIsDisplayTime(bool newIsDisplayTime);

    const QString &timeString() const;
    void setTimeString(const QString &newTimeString);

private:
    bool m_isDisplayTime;
    QString m_timeString;
};

#endif // TIMELINEBLOCKPLAYER_H
