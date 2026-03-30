#ifndef TIMELINEMODELPLAYER_H
#define TIMELINEMODELPLAYER_H

#include <QAbstractListModel>
#include "timelineblockPlayer.h"

class TimelineModelRedact : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit TimelineModelRedact(QObject *parent = nullptr);
    ~TimelineModelRedact();

    enum Roles { TimeStringRole = Qt::UserRole + 1, IsDisplayTimeRole };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addNextMinute();
    Q_INVOKABLE void fillModel(int lastMinute);
    Q_INVOKABLE void clear();

private:
    QList<TimelineBlockRedact *> m_timeLine;
    int m_currentMinute;
};

#endif // TIMELINEMODELPLAYER_H
