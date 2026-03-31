// src/audiosegment.h
//
// Структура данных одного аудиосегмента.
// Сегмент — непрерывный отрезок записи с одним типом звука.
// Сегменты создаются в реальном времени по мере записи
// и хранятся в SegmentListModel для отображения в QML.

#ifndef AUDIOSEGMENT_H
#define AUDIOSEGMENT_H

#include <QtGlobal>

struct AudioSegment
{
    // Три класса звука:
    // Silence — тишина, уровень ниже порога
    // Speech  — человеческая речь (может содержать фоновый шум)
    // Noise   — громкий звук без речи (ветер, машины, музыка)
    enum Type {
        Silence = 0,
        Speech  = 1,
        Noise   = 2
    };

    Type    type;       // Тип сегмента
    qint64  startMs;    // Начало (мс от начала записи)
    qint64  endMs;      // Конец (мс от начала записи)
    float   avgLevel;   // Средний уровень громкости [0.0, 1.0]
    float   peakLevel;  // Пиковый уровень [0.0, 1.0]

    // Длительность сегмента в миллисекундах
    qint64 durationMs() const { return endMs - startMs; }
};

#endif // AUDIOSEGMENT_H
