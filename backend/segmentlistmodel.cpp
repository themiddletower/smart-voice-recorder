// src/segmentlistmodel.cpp

#include "segmentlistmodel.h"

SegmentListModel::SegmentListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int SegmentListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_segments.size();
}

QVariant SegmentListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_segments.size())
        return QVariant();

    const AudioSegment& seg = m_segments[index.row()];

    switch (role) {
    case SegmentTypeRole: return static_cast<int>(seg.type);
    case StartMsRole:     return seg.startMs;
    case EndMsRole:       return seg.endMs;
    case DurationMsRole:  return seg.durationMs();
    case AvgLevelRole:    return static_cast<double>(seg.avgLevel);
    case PeakLevelRole:   return static_cast<double>(seg.peakLevel);
    }
    return QVariant();
}

QHash<int, QByteArray> SegmentListModel::roleNames() const
{
    return {
        { SegmentTypeRole, "segmentType" },
        { StartMsRole,     "startMs" },
        { EndMsRole,       "endMs" },
        { DurationMsRole,  "durationMs" },
        { AvgLevelRole,    "avgLevel" },
        { PeakLevelRole,   "peakLevel" }
    };
}

void SegmentListModel::addSegment(const AudioSegment& segment)
{
    beginInsertRows(QModelIndex(), m_segments.size(), m_segments.size());
    m_segments.append(segment);
    endInsertRows();
    emit countChanged();
}

void SegmentListModel::clear()
{
    beginResetModel();
    m_segments.clear();
    endResetModel();
    emit countChanged();
}

const AudioSegment& SegmentListModel::segmentAt(int index) const
{
    return m_segments[index];
}

qint64 SegmentListModel::totalDurationMs() const
{
    if (m_segments.isEmpty()) return 0;
    return m_segments.last().endMs;
}

qint64 SegmentListModel::speechDurationMs() const
{
    qint64 t = 0;
    for (const auto& s : m_segments)
        if (s.type == AudioSegment::Speech) t += s.durationMs();
    return t;
}

qint64 SegmentListModel::silenceDurationMs() const
{
    qint64 t = 0;
    for (const auto& s : m_segments)
        if (s.type == AudioSegment::Silence) t += s.durationMs();
    return t;
}

qint64 SegmentListModel::noiseDurationMs() const
{
    qint64 t = 0;
    for (const auto& s : m_segments)
        if (s.type == AudioSegment::Noise) t += s.durationMs();
    return t;
}

int SegmentListModel::nextSpeechSegment(int currentIndex) const
{
    for (int i = currentIndex + 1; i < m_segments.size(); i++)
        if (m_segments[i].type == AudioSegment::Speech) return i;
    return -1;
}

int SegmentListModel::prevSpeechSegment(int currentIndex) const
{
    for (int i = currentIndex - 1; i >= 0; i--)
        if (m_segments[i].type == AudioSegment::Speech) return i;
    return -1;
}

qint64 SegmentListModel::segmentStartMs(int index) const
{
    if (index < 0 || index >= m_segments.size()) return 0;
    return m_segments[index].startMs;
}
