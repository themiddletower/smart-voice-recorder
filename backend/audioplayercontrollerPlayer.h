// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AUDIOPLAYERCONTROLLERPLAYER_H
#define AUDIOPLAYERCONTROLLERPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioDecoder>
#include <QAudioBuffer>

#include "audioamplitudemodelPlayer.h"
#include "timelinemodelPlayer.h"

class AudioPlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool isPlayerPage READ isPlayerPage WRITE setIsPlayerPage NOTIFY isPlayerPageChanged)
    Q_PROPERTY(bool isPlaybackAvailable READ isPlaybackAvailable WRITE setIsPlaybackAvailable NOTIFY isPlaybackAvailableChanged)
    Q_PROPERTY(bool isDecoding READ isDecoding NOTIFY isDecodingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(AudioAmplitudeModel *audioAmplitudeModel READ audioAmplitudeModel NOTIFY audioAmplitudeModelChanged)
    Q_PROPERTY(TimelineModel *timelineModel READ timelineModel NOTIFY timelineModelChanged)

public:
    explicit AudioPlayerController(QObject *parent = nullptr);

    Q_INVOKABLE void play(quint64 posInMillis = 0);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void setSource(QString path);
    Q_INVOKABLE QString pointerPositionToString(qint64 position);

    bool isPlaying() const;
    bool isPlayerPage() const;
    void setIsPlayerPage(bool newIsPlayerPage);

    bool isPlaybackAvailable() const;
    void setIsPlaybackAvailable(bool newIsPlaybackAvailable);

    bool isDecoding() const;
    qint64 position() const;

    AudioAmplitudeModel *audioAmplitudeModel();
    TimelineModel *timelineModel();

signals:
    void positionChanged(qint64 position);
    void isPlayingChanged();
    void isPlayerPageChanged();
    void audioAmplitudeModelChanged();
    void timelineModelChanged();
    void isPlaybackAvailableChanged();
    void isDecodingChanged();
    void decodingCompleted();
    void durationChanged(qint64 duration);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 position);

    void onDecoderBufferReady();
    void onDecoderFinished();

private:
    QMediaPlayer m_player;
    QAudioDecoder m_decoder;

    AudioAmplitudeModel m_audioAmplitudeModel;
    TimelineModel m_timelineModel;

    bool m_isPlaying;
    bool m_isPlayerPage;
    bool m_isPlaybackAvailable;
    bool m_isDecoding;

    // --- ВАЖНО: состояние для "непрерывного" подсчёта амплитуд ---
    int m_measurementsPerSec = 10;

    int m_sampleRate = 0;
    int m_channels = 0;
    int m_sampleSize = 0;
    QAudioFormat::SampleType m_sampleType = QAudioFormat::Unknown;
    QAudioFormat::Endian m_endian = QAudioFormat::LittleEndian;

    int m_framesPerPoint = 0;     // sampleRate / measurementsPerSec
    int m_framesAccumulated = 0;  // сколько фреймов собрали в текущую точку
    quint32 m_peakAccumulated = 0;

    quint32 m_maxAllowedAmplitude = 1;

    QList<qreal> m_tempAmplitudes;

    void resetDecodeState();
    quint32 absSampleAsUInt(const unsigned char *p) const;
};

Q_DECLARE_METATYPE(AudioAmplitudeModel *)
Q_DECLARE_METATYPE(TimelineModel *)

#endif // AUDIOPLAYERCONTROLLERPLAYER_H
