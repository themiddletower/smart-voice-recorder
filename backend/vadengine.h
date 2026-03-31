// src/vadengine.h
//
// Обёртка над libfvad.
// Принимает один фрейм PCM int16 → возвращает класс (Silence/Speech/Noise).
//
// libfvad даёт бинарный ответ speech/non-speech.
// Мы добавляем:
//   - RMS для разделения non-speech на Silence и Noise
//   - Адаптивный шумовой пол
//   - Дополнительный hangover (склеивание пауз между словами)

#ifndef VADENGINE_H
#define VADENGINE_H

#include "audiosegment.h"
#include <cstdint>

extern "C" {
#include "fvad.h"
}

class VadEngine
{
public:
    struct FrameResult {
        AudioSegment::Type type;
        float rmsLevel;   // [0.0, 1.0]
        float rmsDb;      // децибелы (обычно от -96 до 0)
    };

    VadEngine();
    ~VadEngine();

    VadEngine(const VadEngine&) = delete;
    VadEngine& operator=(const VadEngine&) = delete;

    FrameResult processFrame(const int16_t* samples, int sampleCount);
    void reset();

    void setSampleRate(int rate);
    void setMode(int mode);
    void setSilenceThresholdDb(float db);
    void setExtraHangoverFrames(int frames);

private:
    float calculateRMS(const int16_t* samples, int count);
    void updateNoiseFloor(float rmsDb, bool isSpeech);
    AudioSegment::Type applySmoothing(bool fvadSpeech, float rmsDb);

    Fvad* m_fvad;

    int   m_sampleRate;
    int   m_mode;
    float m_silenceThresholdDb;
    int   m_extraHangoverFrames;

    bool  m_inSpeech;
    int   m_hangoverCounter;

    float m_noiseFloorDb;
    bool  m_noiseFloorReady;
    int   m_silenceFrameCount;
};

#endif // VADENGINE_H
