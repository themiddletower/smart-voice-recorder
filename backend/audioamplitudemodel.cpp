#include "audioamplitudemodel.h"

AudioAmplitudeModel::AudioAmplitudeModel(QObject *parent) : QAbstractListModel(parent) {}

AudioAmplitudeModel::~AudioAmplitudeModel()
{
    qDeleteAll(m_audioAmplitudes);
}

int AudioAmplitudeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_audioAmplitudes.size();
}

QVariant AudioAmplitudeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {
    case ValueRole:
        return m_audioAmplitudes.at(index.row())->value();
    case IsDefaultValueRole:
        return m_audioAmplitudes.at(index.row())->isDefaultValue();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AudioAmplitudeModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ValueRole] = "value";
    roles[IsDefaultValueRole] = "isDefaultValue";
    return roles;
}

void AudioAmplitudeModel::add(qreal amplitudeValue)
{
    beginInsertRows(QModelIndex(), m_audioAmplitudes.size(), m_audioAmplitudes.size());
    m_audioAmplitudes.append(new AudioAmplitude(amplitudeValue));
    endInsertRows();
}

void AudioAmplitudeModel::fillModel(int elementsCount)
{
    beginInsertRows(QModelIndex(), m_audioAmplitudes.size(),
                    m_audioAmplitudes.size() + elementsCount - 1);
    for (int i = 0; i < elementsCount; i++)
        m_audioAmplitudes.append(new AudioAmplitude(0.05, true));
    endInsertRows();
}

void AudioAmplitudeModel::setValueForElement(qreal amplitudeValue, int index)
{
    if (index < m_audioAmplitudes.size() && m_audioAmplitudes[index]->isDefaultValue()) {
        m_audioAmplitudes[index]->setIsDefaultValue(false);
        m_audioAmplitudes[index]->setValue(amplitudeValue);
        QModelIndex modelIndex = createIndex(index, 0, static_cast<void *>(0));
        emit dataChanged(modelIndex, modelIndex);
    }
}

void AudioAmplitudeModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    qDeleteAll(m_audioAmplitudes);
    m_audioAmplitudes.clear();
    endRemoveRows();
}
