#include "audioamplitudePlayer.h"

AudioAmplitudeRedact::AudioAmplitudeRedact(qreal value, int annotationType)
{
    setValue(value);
    setIsDefaultValue(true);
    setAnnotationType(annotationType);
}

qreal AudioAmplitudeRedact::value() const { return m_value; }
void AudioAmplitudeRedact::setValue(qreal newValue) { m_value = newValue; }

bool AudioAmplitudeRedact::isDefaultValue() const { return m_isDefaultValue; }
void AudioAmplitudeRedact::setIsDefaultValue(bool newIsDefaultValue) { m_isDefaultValue = newIsDefaultValue; }

int AudioAmplitudeRedact::annotationType() const { return m_annotationType; }
void AudioAmplitudeRedact::setAnnotationType(int type) { m_annotationType = type; }
