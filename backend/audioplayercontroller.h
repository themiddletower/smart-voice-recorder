#ifndef AUDIOPLAYERCONTROLLER_H
#define AUDIOPLAYERCONTROLLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioProbe>

#include "audioamplitudemodel.h"
#include "timelinemodel.h"

class AudioPlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool isPlayerPage READ isPlayerPage WRITE setIsPlayerPage NOTIFY isPlayerPageChanged)
    Q_PROPERTY(bool isPlaybackAvailable READ isPlaybackAvailable WRITE setIsPlaybackAvailable
                   NOTIFY isPlaybackAvailableChanged)
    Q_PROPERTY(AudioAmplitudeModel *audioAmplitudeModel READ audioAmplitudeModel
                   NOTIFY audioAmplitudeModelChanged)
    Q_PROPERTY(TimelineModel *timelineModel READ timelineModel NOTIFY timelineModelChanged)

public:
    explicit AudioPlayerController(QObject *parent = nullptr);

    Q_INVOKABLE void play(quint64 posInMillis);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setSource(QString path);
    Q_INVOKABLE void resetModels();
    Q_INVOKABLE void updateModelWithRecorderData(qint64 duration, qreal amplitude);
    Q_INVOKABLE QString pointerPositionToString(qint64 position);

    bool isPlaying() const;
    bool isPlayerPage() const;
    void setIsPlayerPage(bool newIsPlayerPage);
    AudioAmplitudeModel *audioAmplitudeModel();
    TimelineModel *timelineModel();
    bool isPlaybackAvailable() const;
    void setIsPlaybackAvailable(bool newIsPlaybackAvailable);

private:
    QMediaPlayer m_player;
    QAudioProbe m_probe;
    AudioAmplitudeModel m_audioAmplitudeModel;
    TimelineModel m_timelineModel;
    bool m_isPlaying;
    bool m_isPlayerPage;
    bool m_isDurationCalculated;
    bool m_isPlaybackAvailable;

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onProbeRecieved(QAudioBuffer buffer);
    void onDurationChanged(qint64 duration);

signals:
    void positionChanged(qint64 position);
    void isPlayingChanged();
    void isPlayerPageChanged();
    void audioAmplitudeModelChanged();
    void timelineModelChanged();
    void isPlaybackAvailableChanged();
};

Q_DECLARE_METATYPE(AudioAmplitudeModel *)
Q_DECLARE_METATYPE(TimelineModel *)

#endif
