#ifndef TIMELINEMODEL_H
#define TIMELINEMODEL_H

#include <QAbstractListModel>

class TimelineBlock
{
public:
    TimelineBlock(const QString &time = "", bool isDisplay = false)
        : m_timeString(time), m_isDisplayTime(isDisplay) {}
    QString timeString() const { return m_timeString; }
    bool isDisplayTime() const { return m_isDisplayTime; }
private:
    QString m_timeString;
    bool m_isDisplayTime;
};

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel();

    enum Roles { TimeStringRole = Qt::UserRole + 1, IsDisplayTimeRole };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addNextMinute();
    Q_INVOKABLE void fillModel(int lastMinute);
    Q_INVOKABLE void clear();

private:
    QList<TimelineBlock *> m_timeLine;
    int m_currentMinute;
};

#endif
