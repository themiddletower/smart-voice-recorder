// src/vadengine.cpp

#include "vadengine.h"
#include <cmath>
#include <algorithm>

VadEngine::VadEngine()
    : m_fvad(nullptr)
    , m_sampleRate(16000)
    , m_mode(1)
    , m_silenceThresholdDb(-45.0f)
    , m_extraHangoverFrames(10)   // 10 × 20мс = 200мс
    , m_inSpeech(false)
    , m_hangoverCounter(0)
    , m_noiseFloorDb(-60.0f)
    , m_noiseFloorReady(false)
    , m_silenceFrameCount(0)
{
    m_fvad = fvad_new();
    if (m_fvad) {
        fvad_set_mode(m_fvad, m_mode);
        fvad_set_sample_rate(m_fvad, m_sampleRate);
    }
}

VadEngine::~VadEngine()
{
    if (m_fvad) {
        fvad_free(m_fvad);
    }
}

// ────────────────────────────────────────────────────────
// Главный метод. Вызывается для каждого 20-мс фрейма.
//
// Пайплайн:
//   1. RMS → dB (громкость)
//   2. fvad_process → speech? (GMM-классификатор внутри)
//   3. Наша логика: speech / silence / noise + hangover
//   4. Обновление оценки шумового пола
// ────────────────────────────────────────────────────────
VadEngine::FrameResult VadEngine::processFrame(
    const int16_t* samples, int sampleCount)
{
    FrameResult result = {};

    // Шаг 1: RMS
    result.rmsLevel = calculateRMS(samples, sampleCount);
    result.rmsDb = 20.0f * std::log10(result.rmsLevel + 1e-10f);

    // Шаг 2: libfvad
    // Внутри: фильтр-банк 6 полос → GMM → голосование → hangover
    int vadResult = 0;
    if (m_fvad) {
        vadResult = fvad_process(
            m_fvad, samples, static_cast<size_t>(sampleCount));
        if (vadResult < 0) vadResult = 0; // ошибка → non-speech
    }

    // Шаг 3: трёхклассовая классификация + доп. hangover
    result.type = applySmoothing(vadResult == 1, result.rmsDb);

    // Шаг 4: адаптация шумового пола
    updateNoiseFloor(result.rmsDb, result.type == AudioSegment::Speech);

    return result;
}

// ────────────────────────────────────────────────────────
// RMS = sqrt( mean( (sample/32768)² ) )
// ────────────────────────────────────────────────────────
float VadEngine::calculateRMS(const int16_t* samples, int count)
{
    if (count <= 0) return 0.0f;
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        double s = static_cast<double>(samples[i]) / 32768.0;
        sum += s * s;
    }
    return static_cast<float>(std::sqrt(sum / count));
}

// ────────────────────────────────────────────────────────
// Обновление оценки фонового шума.
// Работает только когда нет речи.
// Первые 25 non-speech фреймов (500 мс) — инициализация.
// Далее — экспоненциальное скользящее среднее (alpha=0.97).
// ────────────────────────────────────────────────────────
void VadEngine::updateNoiseFloor(float rmsDb, bool isSpeech)
{
    if (isSpeech) return;
    if (rmsDb < -80.0f) return;

    if (!m_noiseFloorReady) {
        m_silenceFrameCount++;
        m_noiseFloorDb += (rmsDb - m_noiseFloorDb) / m_silenceFrameCount;
        if (m_silenceFrameCount >= 25) {
            m_noiseFloorReady = true;
        }
    } else {
        m_noiseFloorDb = 0.97f * m_noiseFloorDb + 0.03f * rmsDb;
    }
}

// ────────────────────────────────────────────────────────
// Сглаживание + разделение non-speech на silence/noise.
//
// fvad=speech → SPEECH
// fvad=non-speech, но мы были в речи → hangover (ещё SPEECH)
// fvad=non-speech, hangover кончился:
//   rmsDb < порог → SILENCE
//   rmsDb ≥ порог → NOISE
// ────────────────────────────────────────────────────────
AudioSegment::Type VadEngine::applySmoothing(
    bool fvadSpeech, float rmsDb)
{
    if (fvadSpeech) {
        m_inSpeech = true;
        m_hangoverCounter = 0;
        return AudioSegment::Speech;
    }

    // Дополнительный hangover поверх libfvad
    if (m_inSpeech) {
        m_hangoverCounter++;
        if (m_hangoverCounter <= m_extraHangoverFrames) {
            return AudioSegment::Speech;
        }
        m_inSpeech = false;
        m_hangoverCounter = 0;
    }

    // Разделяем silence / noise по громкости
    float threshold = m_silenceThresholdDb;
    if (m_noiseFloorReady) {
        threshold = std::max(m_silenceThresholdDb, m_noiseFloorDb + 6.0f);
    }

    return (rmsDb < threshold) ? AudioSegment::Silence : AudioSegment::Noise;
}

void VadEngine::reset()
{
    if (m_fvad) fvad_reset(m_fvad);
    m_inSpeech = false;
    m_hangoverCounter = 0;
    m_noiseFloorDb = -60.0f;
    m_noiseFloorReady = false;
    m_silenceFrameCount = 0;
}

void VadEngine::setSampleRate(int rate)
{
    m_sampleRate = rate;
    if (m_fvad) {
        if (fvad_set_sample_rate(m_fvad, rate) == -1) {
            fvad_set_sample_rate(m_fvad, 16000);
            m_sampleRate = 16000;
        }
    }
}

void VadEngine::setMode(int mode)
{
    m_mode = std::clamp(mode, 0, 3);
    if (m_fvad) fvad_set_mode(m_fvad, m_mode);
}

void VadEngine::setSilenceThresholdDb(float db)
{
    m_silenceThresholdDb = db;
}

void VadEngine::setExtraHangoverFrames(int frames)
{
    m_extraHangoverFrames = std::max(0, frames);
}
