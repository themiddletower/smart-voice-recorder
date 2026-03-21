#include "audioamplitudemodelPlayer.h"

AudioAmplitudeModel::AudioAmplitudeModel(QObject *parent) : QAbstractListModel(parent) {}

AudioAmplitudeModel::~AudioAmplitudeModel() {
    clear();
}

int AudioAmplitudeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_audioAmplitudes.size();
}

QVariant AudioAmplitudeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_audioAmplitudes.size()) return QVariant();

    AudioAmplitude *item = m_audioAmplitudes.at(index.row());
    switch (role) {
    case ValueRole: return item->value();
    case IsDefaultValueRole: return item->isDefaultValue();
    case AnnotationTypeRole: return item->annotationType();
    default: return QVariant();
    }
}

QHash<int, QByteArray> AudioAmplitudeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ValueRole] = "value";
    roles[IsDefaultValueRole] = "isDefaultValue";
    roles[AnnotationTypeRole] = "annotationType";
    return roles;
}

void AudioAmplitudeModel::setAmplitudes(const QList<qreal> &amplitudes)
{
    beginResetModel();
    qDeleteAll(m_audioAmplitudes);
    m_audioAmplitudes.clear();
    for (qreal amp : amplitudes) {
        AudioAmplitude* item = new AudioAmplitude(amp);
        item->setIsDefaultValue(false);
        m_audioAmplitudes.append(item);
    }
    endResetModel();
}

void AudioAmplitudeModel::clear()
{
    beginResetModel();
    qDeleteAll(m_audioAmplitudes);
    m_audioAmplitudes.clear();
    endResetModel();
}

void AudioAmplitudeModel::applyAnnotations(const QVariantList &annotations, int measurementsPerSec)
{
    if (m_audioAmplitudes.isEmpty()) return;

    for(auto amp : m_audioAmplitudes) {
        amp->setAnnotationType(0);
    }

    for (const QVariant &var : annotations) {
        QVariantMap map = var.toMap();
        qint64 t1 = map.value("t1").toLongLong();
        qint64 t2 = map.value("t2").toLongLong();
        int type = map.value("type").toInt();

        int startIndex = static_cast<int>((t1 / 1000.0) * measurementsPerSec);
        int endIndex = static_cast<int>((t2 / 1000.0) * measurementsPerSec);

        startIndex = qMax(0, startIndex);
        endIndex = qMin(static_cast<int>(m_audioAmplitudes.size() - 1), endIndex);

        for (int i = startIndex; i <= endIndex; ++i) {
            m_audioAmplitudes[i]->setAnnotationType(type);
        }
    }
    emit dataChanged(index(0, 0), index(m_audioAmplitudes.size() - 1, 0), {AnnotationTypeRole});
}
