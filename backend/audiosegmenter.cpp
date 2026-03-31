// src/audiosegmenter.cpp
//
// Пайплайн:
//
// WAV-файл
//    │
//    ▼
// WavReader.readFrame()     ← читаем по 20 мс (320 сэмплов при 16 кГц)
//    │
//    ▼
// VadEngine.processFrame()  ← libfvad + RMS → Silence/Speech/Noise
//    │
//    ▼
// Если тип изменился → закрываем предыдущий сегмент,
//                      открываем новый
//    │
//    ▼
// SegmentListModel          ← сегменты доступны в QML

#include "audiosegmenter.h"
#include "segmentlistmodel.h"
#include <QDebug>
#include <QVector>
#include <algorithm>

// Длительность одного фрейма для VAD (мс).
// libfvad принимает фреймы 10, 20 или 30 мс.
// 20 мс — стандартный выбор: достаточное разрешение,
// не слишком много вызовов.
static const int FRAME_MS = 20;

AudioSegmenter::AudioSegmenter(QObject* parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_status("idle")
    , m_progress(0)
    , m_fileDurationMs(0)
    , m_fileSampleRate(0)
    , m_curType(AudioSegment::Silence)
    , m_curStartMs(0)
    , m_curLevelSum(0)
    , m_curPeak(0)
    , m_curFrameCount(0)
{
}

void AudioSegmenter::setSegmentModel(SegmentListModel* model)
{
    m_model = model;
}

// ════════════════════════════════════════════════════════
// processFile — главный метод.
//
// 1. Открываем WAV через WavReader
// 2. Настраиваем VAD под частоту дискретизации файла
// 3. Читаем фрейм за фреймом
// 4. Классифицируем каждый фрейм
// 5. При смене типа — создаём сегмент
// 6. Закрываем последний сегмент
// ════════════════════════════════════════════════════════
void AudioSegmenter::processFile(const QString& filePath)
{
    // ─── Сброс состояния ───
    m_status = "processing";
    m_progress = 0;
    m_errorMessage.clear();
    emit statusChanged();
    emit progressChanged();
    emit errorChanged();

    if (m_model) m_model->clear();

    // ─── Открываем файл ───
    WavReader reader;
    if (!reader.open(filePath)) {
        m_status = "error";
        m_errorMessage = "Не удалось открыть файл: " + filePath;
        emit statusChanged();
        emit errorChanged();
        return;
    }

    // ─── Информация о файле ───
    m_fileSampleRate = reader.sampleRate();
    m_fileDurationMs = reader.durationMs();
    emit fileInfoChanged();

    qDebug() << "Processing:" << filePath
             << m_fileSampleRate << "Hz,"
             << m_fileDurationMs << "ms";

    // ─── Настраиваем VAD ───
    m_vad.reset();
    m_vad.setSampleRate(m_fileSampleRate);

    // Размер фрейма в сэмплах: sampleRate × frameMs / 1000
    // Для 16 кГц и 20 мс: 16000 × 0.02 = 320 сэмплов
    int frameSamples = m_fileSampleRate * FRAME_MS / 1000;

    // libfvad требует определённые длины фреймов.
    // Допустимые для 16 кГц: 160 (10мс), 320 (20мс), 480 (30мс).
    // Мы используем 320.

    // Буфер для одного фрейма
    QVector<int16_t> frameBuffer(frameSamples);

    // ─── Инициализация отслеживания сегментов ───
    m_curType = AudioSegment::Silence;
    m_curStartMs = 0;
    m_curLevelSum = 0;
    m_curPeak = 0;
    m_curFrameCount = 0;

    qint64 totalSamples = reader.totalSamples();
    qint64 samplesProcessed = 0;
    qint64 frameIndex = 0;

    // ─── Основной цикл: читаем и классифицируем ───
    while (true) {
        // Читаем один фрейм
        int samplesRead = reader.readFrame(
            frameBuffer.data(), frameSamples);

        if (samplesRead == 0) break;  // Конец файла

        // Если прочитали меньше фрейма (конец файла) —
        // дополняем нулями, чтобы VAD получил полный фрейм
        if (samplesRead < frameSamples) {
            std::fill(
                frameBuffer.data() + samplesRead,
                frameBuffer.data() + frameSamples,
                static_cast<int16_t>(0));
        }

        // Классифицируем фрейм
        VadEngine::FrameResult result =
            m_vad.processFrame(frameBuffer.data(), frameSamples);

        // Текущее время (мс)
        qint64 currentMs = frameIndex * FRAME_MS;

        // Если тип изменился — закрываем старый сегмент, начинаем новый
        if (result.type != m_curType && frameIndex > 0) {
            closeSegment(currentMs);

            m_curType = result.type;
            m_curStartMs = currentMs;
            m_curLevelSum = 0;
            m_curPeak = 0;
            m_curFrameCount = 0;
        }

        // Первый фрейм — устанавливаем начальный тип
        if (frameIndex == 0) {
            m_curType = result.type;
        }

        // Обновляем статистику текущего сегмента
        m_curFrameCount++;
        m_curLevelSum += result.rmsLevel;
        m_curPeak = std::max(m_curPeak, result.rmsLevel);

        frameIndex++;
        samplesProcessed += samplesRead;

        // Обновляем прогресс (не чаще чем каждые 50 фреймов = 1 сек)
        if (frameIndex % 50 == 0 && totalSamples > 0) {
            int newProgress = static_cast<int>(
                samplesProcessed * 100 / totalSamples);
            if (newProgress != m_progress) {
                m_progress = newProgress;
                emit progressChanged();
            }
        }
    }

    // ─── Закрываем последний сегмент ───
    qint64 totalMs = frameIndex * FRAME_MS;
    closeSegment(totalMs);

    reader.close();

    // ─── Готово ───
    m_progress = 100;
    m_status = "done";
    emit progressChanged();
    emit statusChanged();
    emit finished();

    qDebug() << "Segmentation complete:"
             << (m_model ? m_model->rowCount() : 0) << "segments";
}

// ════════════════════════════════════════════════════════
// closeSegment — финализация текущего сегмента.
// Создаёт AudioSegment и добавляет в модель.
// ════════════════════════════════════════════════════════
void AudioSegmenter::closeSegment(qint64 endMs)
{
    if (m_curFrameCount == 0) return;

    AudioSegment seg;
    seg.type     = m_curType;
    seg.startMs  = m_curStartMs;
    seg.endMs    = endMs;
    seg.avgLevel = m_curLevelSum / m_curFrameCount;
    seg.peakLevel = m_curPeak;

    if (m_model) {
        m_model->addSegment(seg);
    }
}

// ─── Getters ───
QString AudioSegmenter::status() const { return m_status; }
int AudioSegmenter::progress() const { return m_progress; }
QString AudioSegmenter::errorMessage() const { return m_errorMessage; }
qint64 AudioSegmenter::fileDurationMs() const { return m_fileDurationMs; }
int AudioSegmenter::fileSampleRate() const { return m_fileSampleRate; }

// ─── Настройки VAD (проксируем в VadEngine) ───
void AudioSegmenter::setVadMode(int mode) {
    m_vad.setMode(mode);
}

void AudioSegmenter::setSilenceThresholdDb(float db) {
    m_vad.setSilenceThresholdDb(db);
}

void AudioSegmenter::setHangoverMs(int ms) {
    // Переводим мс в количество фреймов
    m_vad.setExtraHangoverFrames(ms / FRAME_MS);
}
