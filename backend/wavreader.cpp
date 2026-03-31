// src/wavreader.cpp
//
// Парсинг WAV-заголовка и покадровое чтение PCM.
//
// Структура WAV (упрощённо):
//
//  0: "RIFF"
//  4: размер файла - 8
//  8: "WAVE"
// 12: "fmt "
// 16: размер fmt-блока (16 для PCM)
// 20: формат (1 = PCM)
// 22: каналы
// 24: частота дискретизации
// 28: байт/сек
// 32: блок-выравнивание
// 34: бит на сэмпл
// 36: "data"
// 40: размер данных
// 44: начало PCM

#include "wavreader.h"
#include <QDataStream>
#include <QDebug>

WavReader::WavReader()
    : m_sampleRate(0)
    , m_channels(0)
    , m_bitsPerSample(0)
    , m_dataSize(0)
    , m_dataOffset(0)
{
}

WavReader::~WavReader()
{
    close();
}

bool WavReader::open(const QString& filePath)
{
    close();

    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << filePath;
        return false;
    }

    if (!parseHeader()) {
        qWarning() << "Invalid WAV header:" << filePath;
        m_file.close();
        return false;
    }

    return true;
}

void WavReader::close()
{
    if (m_file.isOpen()) {
        m_file.close();
    }
}

// ────────────────────────────────────────────────────────
// parseHeader — читаем и проверяем WAV-заголовок.
//
// Ищем chunk "fmt " для параметров формата
// и chunk "data" для начала PCM-данных.
// Некоторые WAV-файлы имеют дополнительные chunks
// между fmt и data (например, "LIST", "INFO"),
// поэтому мы не полагаемся на фиксированные смещения,
// а ищем chunks по маркерам.
// ────────────────────────────────────────────────────────
bool WavReader::parseHeader()
{
    if (m_file.size() < 44) return false;

    QDataStream s(&m_file);
    s.setByteOrder(QDataStream::LittleEndian);

    // ─── RIFF header ───
    char riff[4];
    s.readRawData(riff, 4);
    if (QByteArray(riff, 4) != "RIFF") return false;

    quint32 fileSize;
    s >> fileSize;

    char wave[4];
    s.readRawData(wave, 4);
    if (QByteArray(wave, 4) != "WAVE") return false;

    // ─── Ищем chunks ───
    bool fmtFound = false;
    bool dataFound = false;

    while (!s.atEnd() && !(fmtFound && dataFound)) {
        char chunkId[4];
        quint32 chunkSize;

        if (s.readRawData(chunkId, 4) != 4) break;
        s >> chunkSize;

        QByteArray id(chunkId, 4);

        if (id == "fmt ") {
            // ─── Блок формата ───
            quint16 audioFormat;
            s >> audioFormat;
            if (audioFormat != 1) {
                // Не PCM — не поддерживаем (MP3, ADPCM и т.д.)
                qWarning() << "Unsupported audio format:" << audioFormat;
                return false;
            }

            quint16 channels;
            quint32 sampleRate;
            quint32 byteRate;
            quint16 blockAlign;
            quint16 bitsPerSample;

            s >> channels >> sampleRate >> byteRate
                >> blockAlign >> bitsPerSample;

            m_channels = channels;
            m_sampleRate = sampleRate;
            m_bitsPerSample = bitsPerSample;
            fmtFound = true;

            // Пропускаем остаток fmt-блока (если есть extra bytes)
            qint64 remaining = chunkSize - 16;
            if (remaining > 0) {
                m_file.seek(m_file.pos() + remaining);
            }

        } else if (id == "data") {
            // ─── Блок данных ───
            m_dataSize = chunkSize;
            m_dataOffset = m_file.pos();
            dataFound = true;
            // Не читаем данные — будем читать покадрово

        } else {
            // Неизвестный chunk — пропускаем
            m_file.seek(m_file.pos() + chunkSize);
        }
    }

    if (!fmtFound || !dataFound) return false;

    // Проверяем поддерживаемый формат
    if (m_bitsPerSample != 16) {
        qWarning() << "Only 16-bit WAV supported, got:" << m_bitsPerSample;
        return false;
    }

    // Ставим позицию чтения на начало данных
    m_file.seek(m_dataOffset);

    qDebug() << "WAV opened:" << m_sampleRate << "Hz,"
             << m_channels << "ch,"
             << m_bitsPerSample << "bit,"
             << durationMs() << "ms";

    return true;
}

// ────────────────────────────────────────────────────────
// readFrame — читаем один фрейм PCM-данных.
//
// frameSamples — сколько МОНО-сэмплов нужно (например, 320).
// Если файл стерео — читаем вдвое больше и микшируем:
//   mono = (left + right) / 2
//
// Возвращает реально прочитанных моно-сэмплов (0 = конец).
// ────────────────────────────────────────────────────────
int WavReader::readFrame(int16_t* outputBuffer, int frameSamples)
{
    if (!m_file.isOpen()) return 0;

    if (m_channels == 1) {
        // Моно — читаем напрямую
        int bytesNeeded = frameSamples * 2;  // int16 = 2 байта
        QByteArray raw = m_file.read(bytesNeeded);
        int samplesRead = raw.size() / 2;

        if (samplesRead > 0) {
            memcpy(outputBuffer, raw.constData(), samplesRead * 2);
        }
        return samplesRead;

    } else {
        // Стерео (или больше каналов) — читаем все каналы, микшируем в моно
        int totalSamplesNeeded = frameSamples * m_channels;
        int bytesNeeded = totalSamplesNeeded * 2;

        QByteArray raw = m_file.read(bytesNeeded);
        int totalSamplesRead = raw.size() / 2;
        int monoSamplesRead = totalSamplesRead / m_channels;

        if (monoSamplesRead > 0) {
            const int16_t* src =
                reinterpret_cast<const int16_t*>(raw.constData());

            for (int i = 0; i < monoSamplesRead; i++) {
                // Среднее всех каналов
                int32_t sum = 0;
                for (int ch = 0; ch < m_channels; ch++) {
                    sum += src[i * m_channels + ch];
                }
                outputBuffer[i] = static_cast<int16_t>(sum / m_channels);
            }
        }
        return monoSamplesRead;
    }
}

bool WavReader::isOpen() const { return m_file.isOpen(); }
int WavReader::sampleRate() const { return m_sampleRate; }
int WavReader::channels() const { return m_channels; }
int WavReader::bitsPerSample() const { return m_bitsPerSample; }

qint64 WavReader::totalSamples() const
{
    if (m_bitsPerSample == 0 || m_channels == 0) return 0;
    int bytesPerSample = m_bitsPerSample / 8;
    return m_dataSize / (bytesPerSample * m_channels);
}

qint64 WavReader::durationMs() const
{
    if (m_sampleRate == 0) return 0;
    return totalSamples() * 1000 / m_sampleRate;
}
