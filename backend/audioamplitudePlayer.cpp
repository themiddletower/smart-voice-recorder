// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#include "audioamplitudePlayer.h"

AudioAmplitude::AudioAmplitude(qreal value, int annotationType)
{
    setValue(value);
    setIsDefaultValue(true);
    setAnnotationType(annotationType);
}

qreal AudioAmplitude::value() const { return m_value; }
void AudioAmplitude::setValue(qreal newValue) { m_value = newValue; }

bool AudioAmplitude::isDefaultValue() const { return m_isDefaultValue; }
void AudioAmplitude::setIsDefaultValue(bool newIsDefaultValue) { m_isDefaultValue = newIsDefaultValue; }

int AudioAmplitude::annotationType() const { return m_annotationType; }
void AudioAmplitude::setAnnotationType(int type) { m_annotationType = type; }
