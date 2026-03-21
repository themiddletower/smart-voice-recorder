// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AUDIOAMPLITUDEPLAYER_H
#define AUDIOAMPLITUDEPLAYER_H

#include <QObject>

class AudioAmplitude
{
public:
    AudioAmplitude(qreal value, int annotationType = 0);

    qreal value() const;
    void setValue(qreal newValue);

    bool isDefaultValue() const;
    void setIsDefaultValue(bool newIsDefaultValue);

    int annotationType() const;
    void setAnnotationType(int type);

private:
    qreal m_value;
    bool m_isDefaultValue;
    int m_annotationType; // 0 - default, 1 - voice, 2 - silence, 3 - noise
};

#endif // AUDIOAMPLITUDEPLAYER_H
