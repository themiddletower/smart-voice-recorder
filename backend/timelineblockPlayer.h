#ifndef TIMELINEBLOCKPLAYER_H
#define TIMELINEBLOCKPLAYER_H

#include <QString>

class TimelineBlockRedact
{
public:
    TimelineBlockRedact(QString timeString, bool isDisplayTime);

    bool isDisplayTime() const;
    void setIsDisplayTime(bool newIsDisplayTime);

    const QString &timeString() const;
    void setTimeString(const QString &newTimeString);

private:
    bool m_isDisplayTime;
    QString m_timeString;
};

#endif // TIMELINEBLOCKPLAYER_H
