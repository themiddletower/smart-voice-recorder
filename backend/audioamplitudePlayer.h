#ifndef AUDIOAMPLITUDEPLAYER_H
#define AUDIOAMPLITUDEPLAYER_H

#include <QObject>

class AudioAmplitudeRedact
{
public:
    AudioAmplitudeRedact(qreal value = 0.05, int annotationType = 0);

    qreal value() const;
    void setValue(qreal newValue);

    bool isDefaultValue() const;
    void setIsDefaultValue(bool newIsDefaultValue);

    int annotationType() const;
    void setAnnotationType(int type);

private:
    qreal m_value;
    bool m_isDefaultValue;
    int m_annotationType;
};

#endif // AUDIOAMPLITUDEPLAYER_H
