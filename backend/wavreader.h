// src/wavreader.h
//
// Чтение WAV-файлов. Парсит 44-байтовый заголовок,
// затем отдаёт PCM-данные фреймами нужного размера.
//
// Поддерживает только PCM (без сжатия), моно/стерео, 16 бит.
// Стерео автоматически микшируется в моно при чтении.

#ifndef WAVREADER_H
#define WAVREADER_H

#include <QFile>
#include <QString>
#include <QVector>
#include <cstdint>

class WavReader
{
public:
    WavReader();
    ~WavReader();

    // Открыть WAV-файл и прочитать заголовок
    bool open(const QString& filePath);
    void close();

    // Прочитать следующий фрейм (frameSamples сэмплов моно int16).
    // Возвращает количество реально прочитанных сэмплов.
    // 0 = конец файла.
    int readFrame(int16_t* outputBuffer, int frameSamples);

    // Метаданные
    bool isOpen() const;
    int sampleRate() const;
    int channels() const;
    int bitsPerSample() const;
    qint64 totalSamples() const;       // Общее кол-во сэмплов (моно)
    qint64 durationMs() const;         // Длительность файла в мс

private:
    bool parseHeader();

    QFile   m_file;
    int     m_sampleRate;
    int     m_channels;
    int     m_bitsPerSample;
    qint64  m_dataSize;        // Размер PCM-данных в байтах
    qint64  m_dataOffset;      // Смещение начала PCM-данных в файле

    // Буфер для чтения стерео (чтобы потом смикшировать в моно)
    QVector<int16_t> m_readBuffer;
};

#endif // WAVREADER_H
