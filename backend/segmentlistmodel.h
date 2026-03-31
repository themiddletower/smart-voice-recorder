// src/segmentlistmodel.h
//
// Модель сегментов для QML.

#ifndef SEGMENTLISTMODEL_H
#define SEGMENTLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "audiosegment.h"

class SegmentListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(qint64 totalDurationMs READ totalDurationMs NOTIFY countChanged)
    Q_PROPERTY(qint64 speechDurationMs READ speechDurationMs NOTIFY countChanged)
    Q_PROPERTY(qint64 silenceDurationMs READ silenceDurationMs NOTIFY countChanged)
    Q_PROPERTY(qint64 noiseDurationMs READ noiseDurationMs NOTIFY countChanged)

public:
    enum Roles {
        SegmentTypeRole = Qt::UserRole + 1,
        StartMsRole,
        EndMsRole,
        DurationMsRole,
        AvgLevelRole,
        PeakLevelRole
    };

    explicit SegmentListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addSegment(const AudioSegment& segment);
    void clear();
    const AudioSegment& segmentAt(int index) const;

    qint64 totalDurationMs() const;
    qint64 speechDurationMs() const;
    qint64 silenceDurationMs() const;
    qint64 noiseDurationMs() const;

    Q_INVOKABLE int nextSpeechSegment(int currentIndex) const;
    Q_INVOKABLE int prevSpeechSegment(int currentIndex) const;
    Q_INVOKABLE qint64 segmentStartMs(int index) const;

signals:
    void countChanged();

private:
    QVector<AudioSegment> m_segments;
};

#endif // SEGMENTLISTMODEL_H
