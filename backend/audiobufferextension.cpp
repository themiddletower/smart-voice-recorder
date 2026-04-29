#include <qendian.h>
#include "audiobufferextension.h"

const static qreal s_minimalAmplitude = 0.05;

namespace AudioBufferExtension {

qreal calculateAmplitude(QAudioBuffer buffer)
{
    QAudioFormat format = buffer.format();
    QAudioFormat::SampleType sampleType = format.sampleType();
    QAudioFormat::Endian endianType = format.byteOrder();
    int sampleSize = format.sampleSize();
    const unsigned char *ptr = buffer.constData<unsigned char>();
    quint32 amplitude = 0;
    quint32 maxAmplitude = UINT8_MAX;

    if (sampleSize == 8) {
        if (sampleType == QAudioFormat::UnSignedInt)
            amplitude = *reinterpret_cast<const quint8 *>(ptr);
        else if (sampleType == QAudioFormat::SignedInt)
            amplitude = qAbs(*reinterpret_cast<const qint8 *>(ptr));
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
    if (currentLevel > 1)
        currentLevel = 1;
    else if (currentLevel < s_minimalAmplitude)
        currentLevel = s_minimalAmplitude;

    return currentLevel;
}

}
