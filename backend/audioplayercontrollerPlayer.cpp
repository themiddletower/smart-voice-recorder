// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#include "audioplayercontrollerPlayer.h"

#include <qendian.h>
#include <QtMath>
#include <QUrl>

static const int s_listenIntervalInMillis = 20;
static const qreal s_minimalAmplitude = 0.05;

AudioPlayerController::AudioPlayerController(QObject *parent)
    : QObject(parent)
    , m_isPlaying(false)
    , m_isPlayerPage(false)
    , m_isPlaybackAvailable(false)
    , m_isDecoding(false)
{
    m_player.setNotifyInterval(s_listenIntervalInMillis);

    connect(&m_player, &QMediaPlayer::mediaStatusChanged,
            this, &AudioPlayerController::onMediaStatusChanged);
    connect(&m_player, &QMediaPlayer::positionChanged,
            this, &AudioPlayerController::onPositionChanged);

    connect(&m_player, &QMediaPlayer::durationChanged, this, [this](qint64 d) {
        emit durationChanged(d);
    });

    connect(&m_decoder, &QAudioDecoder::bufferReady,
            this, &AudioPlayerController::onDecoderBufferReady);
    connect(&m_decoder, &QAudioDecoder::finished,
            this, &AudioPlayerController::onDecoderFinished);

    resetDecodeState();
}

void AudioPlayerController::resetDecodeState()
{
    m_sampleRate = 0;
    m_channels = 0;
    m_sampleSize = 0;
    m_sampleType = QAudioFormat::Unknown;
    m_endian = QAudioFormat::LittleEndian;

    m_framesPerPoint = 0;
    m_framesAccumulated = 0;
    m_peakAccumulated = 0;
    m_maxAllowedAmplitude = 1;

    m_tempAmplitudes.clear();
}

void AudioPlayerController::play(quint64 posInMillis)
{
    if (posInMillis != static_cast<quint64>(m_player.position())) {
        m_player.setPosition(posInMillis);
    }
    m_player.play();
    m_isPlaying = true;
    emit isPlayingChanged();
}

void AudioPlayerController::stop()
{
    m_player.stop(); // ИСПРАВЛЕНО: теперь stop() действительно останавливает
    m_isPlaying = false;
    emit isPlayingChanged();
}

// ДОБАВЛЕНО: реализация функции паузы
void AudioPlayerController::pause()
{
    m_player.pause(); // Ставим на паузу без сброса позиции
    m_isPlaying = false;
    emit isPlayingChanged();
}

void AudioPlayerController::setSource(QString path)
{
    m_player.setMedia(QUrl::fromLocalFile(path));

    m_audioAmplitudeModel.clear();
    m_timelineModel.clear();
    resetDecodeState();

    if (m_isPlayerPage) {
        if (m_decoder.state() != QAudioDecoder::StoppedState)
            m_decoder.stop();

        m_decoder.setSourceFilename(path);
        m_isDecoding = true;
        emit isDecodingChanged();
        m_decoder.start();
    }
}

TimelineModel *AudioPlayerController::timelineModel()
{
    return &m_timelineModel;
}

quint32 AudioPlayerController::absSampleAsUInt(const unsigned char *p) const
{
    quint32 amp = 0;

    if (m_sampleSize == 8) {
        if (m_sampleType == QAudioFormat::UnSignedInt) {
            amp = *reinterpret_cast<const quint8 *>(p);
        } else if (m_sampleType == QAudioFormat::SignedInt) {
            amp = qAbs(*reinterpret_cast<const qint8 *>(p));
        }
        return amp;
    }

    if (m_sampleSize == 16) {
        if (m_sampleType == QAudioFormat::UnSignedInt) {
            amp = (m_endian == QAudioFormat::LittleEndian) ? qFromLittleEndian<quint16>(p)
                                                           : qFromBigEndian<quint16>(p);
        } else if (m_sampleType == QAudioFormat::SignedInt) {
            amp = (m_endian == QAudioFormat::LittleEndian) ? quint32(qAbs(qFromLittleEndian<qint16>(p)))
                                                           : quint32(qAbs(qFromBigEndian<qint16>(p)));
        }
        return amp;
    }

    if (m_sampleSize == 32) {
        if (m_sampleType == QAudioFormat::UnSignedInt) {
            amp = (m_endian == QAudioFormat::LittleEndian) ? qFromLittleEndian<quint32>(p)
                                                           : qFromBigEndian<quint32>(p);
        } else if (m_sampleType == QAudioFormat::SignedInt) {
            amp = (m_endian == QAudioFormat::LittleEndian) ? quint32(qAbs(qFromLittleEndian<qint32>(p)))
                                                           : quint32(qAbs(qFromBigEndian<qint32>(p)));
        } else if (m_sampleType == QAudioFormat::Float) {
            amp = quint32(qAbs(*reinterpret_cast<const float *>(p)) * 0x7fffffff);
        }
        return amp;
    }

    return 0;
}

void AudioPlayerController::onDecoderBufferReady()
{
    const QAudioBuffer buffer = m_decoder.read();
    if (!buffer.isValid())
        return;

    const QAudioFormat fmt = buffer.format();
    if (!fmt.isValid())
        return;

    if (m_sampleRate == 0) {
        m_sampleRate = fmt.sampleRate();
        m_channels = fmt.channelCount();
        m_sampleSize = fmt.sampleSize();
        m_sampleType = fmt.sampleType();
        m_endian = fmt.byteOrder();

        m_framesPerPoint = qMax(1, m_sampleRate / m_measurementsPerSec);

        if (m_sampleSize == 8) m_maxAllowedAmplitude = UINT8_MAX;
        else if (m_sampleSize == 16) m_maxAllowedAmplitude = UINT16_MAX;
        else if (m_sampleSize == 32) m_maxAllowedAmplitude = UINT32_MAX;
        else m_maxAllowedAmplitude = 1;
    }

    const int bytesPerSample = m_sampleSize / 8;
    if (bytesPerSample <= 0 || m_channels <= 0)
        return;

    const unsigned char *ptr = buffer.constData<unsigned char>();

    const int frames = buffer.frameCount();
    for (int f = 0; f < frames; ++f) {
        quint32 peakInFrame = 0;

        for (int ch = 0; ch < m_channels; ++ch) {
            const int sampleIndex = (f * m_channels + ch) * bytesPerSample;
            const unsigned char *sp = ptr + sampleIndex;
            const quint32 a = absSampleAsUInt(sp);
            if (a > peakInFrame) peakInFrame = a;
        }

        if (peakInFrame > m_peakAccumulated)
            m_peakAccumulated = peakInFrame;

        m_framesAccumulated++;

        if (m_framesAccumulated >= m_framesPerPoint) {
            qreal level = (qreal)m_peakAccumulated / (qreal)m_maxAllowedAmplitude * 2.0;
            level = qBound<qreal>(s_minimalAmplitude, level, 1.0);

            m_tempAmplitudes.append(level);

            m_framesAccumulated = 0;
            m_peakAccumulated = 0;
        }
    }
}

void AudioPlayerController::onDecoderFinished()
{
    m_isDecoding = false;
    emit isDecodingChanged();

    const qint64 durMs = m_player.duration();
    if (durMs > 0) {
        const int expected = qMax(1, int(qCeil((durMs / 1000.0) * m_measurementsPerSec)));

        if (m_tempAmplitudes.size() > expected) {
            m_tempAmplitudes = m_tempAmplitudes.mid(0, expected);
        } else if (m_tempAmplitudes.size() < expected) {
            const qreal last = m_tempAmplitudes.isEmpty() ? s_minimalAmplitude : m_tempAmplitudes.last();
            while (m_tempAmplitudes.size() < expected)
                m_tempAmplitudes.append(last);
        }

        m_timelineModel.fillModel(int(durMs / 60000));
    } else {
        const qint64 totalMs = qint64((m_tempAmplitudes.size() / double(m_measurementsPerSec)) * 1000.0);
        m_timelineModel.fillModel(int(totalMs / 60000));
    }

    m_audioAmplitudeModel.setAmplitudes(m_tempAmplitudes);

    emit decodingCompleted();
}

QString AudioPlayerController::pointerPositionToString(qint64 position)
{
    QString millis = QString::number((position / 10) % 100).rightJustified(2, '0');
    QString seconds = QString::number((position / 1000) % 60).rightJustified(2, '0');
    QString mins = QString::number(position / 60000).rightJustified(2, '0');
    return mins + ":" + seconds + "." + millis;
}

bool AudioPlayerController::isPlaying() const { return m_isPlaying; }
bool AudioPlayerController::isPlayerPage() const { return m_isPlayerPage; }

void AudioPlayerController::setIsPlayerPage(bool newIsPlayerPage)
{
    if (m_isPlayerPage == newIsPlayerPage) return;
    m_isPlayerPage = newIsPlayerPage;
    emit isPlayerPageChanged();
}

bool AudioPlayerController::isPlaybackAvailable() const { return m_isPlaybackAvailable; }

void AudioPlayerController::setIsPlaybackAvailable(bool newIsPlaybackAvailable)
{
    if (m_isPlaybackAvailable == newIsPlaybackAvailable) return;
    m_isPlaybackAvailable = newIsPlaybackAvailable;
    emit isPlaybackAvailableChanged();
}

bool AudioPlayerController::isDecoding() const { return m_isDecoding; }
qint64 AudioPlayerController::position() const { return m_player.position(); }

AudioAmplitudeModel *AudioPlayerController::audioAmplitudeModel()
{
    return &m_audioAmplitudeModel;
}

void AudioPlayerController::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia) {
        setIsPlaybackAvailable(true);
    } else if (status == QMediaPlayer::InvalidMedia) {
        setIsPlaybackAvailable(false);
    } else if (status == QMediaPlayer::EndOfMedia) {
        m_isPlaying = false;
        emit isPlayingChanged();
    }
}

void AudioPlayerController::onPositionChanged(qint64 pos)
{
    emit positionChanged(pos);
}
