// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#include <qendian.h>
#include <QDebug>
#include "audiobufferextensionPlayer.h"

const static qreal s_minimalAmplitude = 0.05;

namespace AudioBufferExtension {

// --- Функция для Плеера (массив значений) ---
QList<qreal> calculateAmplitudes(const QAudioBuffer &buffer, int measurementsPerSec)
{
    QList<qreal> resultList;
    QAudioFormat format = buffer.format();
    if (!format.isValid()) return resultList;

    int sampleRate = format.sampleRate();
    int channels = format.channelCount();
    int sampleSize = format.sampleSize();
    QAudioFormat::SampleType sampleType = format.sampleType();
    QAudioFormat::Endian endianType = format.byteOrder();

    int bytesPerSample = sampleSize / 8;
    int samplesPerChunk = sampleRate / measurementsPerSec;
    int bytesPerChunk = samplesPerChunk * channels * bytesPerSample;

    if (bytesPerChunk == 0) return resultList;

    const unsigned char *ptr = buffer.constData<unsigned char>();
    int totalBytes = buffer.byteCount();

    quint32 maxAllowedAmplitude = (sampleSize == 8) ? UINT8_MAX : (sampleSize == 16) ? UINT16_MAX : UINT32_MAX;

    for (int offset = 0; offset < totalBytes; offset += bytesPerChunk) {
        int limit = qMin(offset + bytesPerChunk, totalBytes);
        quint32 peakAmplitude = 0;

        for (int i = offset; i < limit; i += bytesPerSample * channels) {
            quint32 currentAmp = 0;
            const unsigned char *samplePtr = ptr + i;

            if (sampleSize == 8) {
                if (sampleType == QAudioFormat::UnSignedInt) {
                    currentAmp = *reinterpret_cast<const quint8 *>(samplePtr);
                } else if (sampleType == QAudioFormat::SignedInt) {
                    currentAmp = qAbs(*reinterpret_cast<const qint8 *>(samplePtr));
                }
            } else if (sampleSize == 16) {
                if (sampleType == QAudioFormat::UnSignedInt) {
                    currentAmp = (endianType == QAudioFormat::LittleEndian) ? qFromLittleEndian<quint16>(samplePtr) : qFromBigEndian<quint16>(samplePtr);
                } else if (sampleType == QAudioFormat::SignedInt) {
                    currentAmp = qAbs((endianType == QAudioFormat::LittleEndian) ? qFromLittleEndian<qint16>(samplePtr) : qFromBigEndian<qint16>(samplePtr));
                }
            } else if (sampleSize == 32) {
                if (sampleType == QAudioFormat::UnSignedInt) {
                    currentAmp = (endianType == QAudioFormat::LittleEndian) ? qFromLittleEndian<quint32>(samplePtr) : qFromBigEndian<quint32>(samplePtr);
                } else if (sampleType == QAudioFormat::SignedInt) {
                    currentAmp = qAbs((endianType == QAudioFormat::LittleEndian) ? qFromLittleEndian<qint32>(samplePtr) : qFromBigEndian<qint32>(samplePtr));
                } else if (sampleType == QAudioFormat::Float) {
                    currentAmp = qAbs(*reinterpret_cast<const float *>(samplePtr) * 0x7fffffff);
                }
            }
            if (currentAmp > peakAmplitude) {
                peakAmplitude = currentAmp;
            }
        }

        qreal level = (qreal)peakAmplitude / maxAllowedAmplitude * 2.0;
        level = qBound<qreal>(s_minimalAmplitude, level, 1.0);
        resultList.append(level);
    }

    return resultList;
}

// --- Старая функция для Диктофона (одно значение) ---
qreal calculateAmplitude(const QAudioBuffer &buffer)
{
    QAudioFormat format = buffer.format();
    QAudioFormat::SampleType sampleType = format.sampleType();
    QAudioFormat::Endian endianType = format.byteOrder();
    int sampleSize = format.sampleSize();
    const unsigned char *ptr = buffer.constData<unsigned char>();
    quint32 amplitude = 0;
    quint32 maxAmplitude = UINT8_MAX;

    if (sampleSize == 8) {
        if (sampleType == QAudioFormat::UnSignedInt) {
            amplitude = *reinterpret_cast<const quint8 *>(ptr);
        } else if (sampleType == QAudioFormat::SignedInt) {
            amplitude = qAbs(*reinterpret_cast<const qint8 *>(ptr));
        }
    } else if (sampleSize == 16) {
        maxAmplitude = UINT16_MAX;
        if (sampleType == QAudioFormat::UnSignedInt) {
            if (endianType == QAudioFormat::LittleEndian)
                amplitude = qFromLittleEndian<quint16>(ptr);
            else
                amplitude = qFromBigEndian<quint16>(ptr);
        } else if (sampleType == QAudioFormat::SignedInt) {
            if (endianType == QAudioFormat::LittleEndian)
                amplitude = qAbs(qFromLittleEndian<qint16>(ptr));
            else
                amplitude = qAbs(qFromBigEndian<qint16>(ptr));
        }
    } else if (sampleSize == 32) {
        maxAmplitude = UINT32_MAX;
        if (sampleType == QAudioFormat::UnSignedInt) {
            if (endianType == QAudioFormat::LittleEndian)
                amplitude = qFromLittleEndian<quint32>(ptr);
            else
                amplitude = qFromBigEndian<quint32>(ptr);
        } else if (sampleType == QAudioFormat::SignedInt) {
            if (endianType == QAudioFormat::LittleEndian)
                amplitude = qAbs(qFromLittleEndian<qint32>(ptr));
            else
                amplitude = qAbs(qFromBigEndian<qint32>(ptr));
        } else {
            amplitude = qAbs(*reinterpret_cast<const float *>(ptr) * 0x7fffffff);
        }
    }

    qreal currentLevel = (qreal)amplitude / maxAmplitude * 10;
    return qBound<qreal>(s_minimalAmplitude, currentLevel, 1.0);
}

} // namespace AudioBufferExtension
