#ifndef AUDIOAMPLITUDEMODEL_H
#define AUDIOAMPLITUDEMODEL_H

#include <QAbstractListModel>
#include <QThread>

class AudioAmplitude
{
public:
    AudioAmplitude(qreal value = 0.05, bool isDefault = false)
        : m_value(value), m_isDefaultValue(isDefault) {}
    qreal value() const { return m_value; }
    void setValue(qreal v) { m_value = v; }
    bool isDefaultValue() const { return m_isDefaultValue; }
    void setIsDefaultValue(bool d) { m_isDefaultValue = d; }
private:
    qreal m_value;
    bool m_isDefaultValue;
};

class AudioAmplitudeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AudioAmplitudeModel(QObject *parent = nullptr);
    ~AudioAmplitudeModel();

    enum Roles { ValueRole = Qt::UserRole + 1, IsDefaultValueRole };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void add(qreal amplitudeValue);
    Q_INVOKABLE void fillModel(int elementsCount);
    Q_INVOKABLE void setValueForElement(qreal amplitudeValue, int index);
    Q_INVOKABLE void clear();

private:
    QList<AudioAmplitude *> m_audioAmplitudes;
};

#endif
