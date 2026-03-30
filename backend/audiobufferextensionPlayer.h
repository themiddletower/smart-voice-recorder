// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AUDIOBUFFEREXTENTION_H
#define AUDIOBUFFEREXTENTION_H

#include <QAudioBuffer>
#include <QList>

namespace AudioBufferExtension {
// Для плеера (разбирает большой кусок данных и отдает массив амплитуд)
QList<qreal> calculateAmplitudes(const QAudioBuffer &buffer, int measurementsPerSec = 16);

// Для диктофона (возвращает одну амплитуду текущего момента времени)
qreal calculateAmplitude(const QAudioBuffer &buffer);
}

#endif // AUDIOBUFFEREXTENTION_H
