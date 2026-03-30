#include <qendian.h>
#include "audioplayercontroller.h"
#include "audiobufferextension.h"

const static int s_millisInMin = 60000;
const static int s_millisInSec = 1000;
const static int s_measurementsPerSec = 16;
const static int s_listenIntervalInMillis = 20;

AudioPlayerController::AudioPlayerController(QObject *parent)
    : QObject(parent), m_isPlaying(false), m_isPlayerPage(false),
    m_isDurationCalculated(false), m_isPlaybackAvailable(false)
{
    m_player.setNotifyInterval(s_listenIntervalInMillis);
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this,
            &AudioPlayerController::onMediaStatusChanged);
    connect(&m_player, &QMediaPlayer::durationChanged, this,
            &AudioPlayerController::onDurationChanged);
    connect(&m_player, &QMediaPlayer::positionChanged, this,
            &AudioPlayerController::positionChanged);
    connect(&m_probe, &QAudioProbe::audioBufferProbed, this,
            &AudioPlayerController::onProbeRecieved);
}

void AudioPlayerController::play(quint64 posInMillis)
{
    m_player.setMedia(m_player.media());
    m_player.setPosition(posInMillis);
    m_player.play();
    m_isPlaying = true;
    emit isPlayingChanged();
}

void AudioPlayerController::stop()
{
    m_player.stop();
    m_isPlaying = false;
    emit isPlayingChanged();
}

void AudioPlayerController::setSource(QString path)
{
    m_isDurationCalculated = false;
    m_player.setMedia(QUrl::fromLocalFile(path));
    if (m_isPlayerPage)
        m_probe.setSource(&m_player);
}

void AudioPlayerController::resetModels()
{
    m_timelineModel.clear();
    m_timelineModel.addNextMinute();
    m_audioAmplitudeModel.clear();
}

void AudioPlayerController::updateModelWithRecorderData(qint64 duration, qreal amplitude)
{
    m_audioAmplitudeModel.add(amplitude);
    if (duration > s_millisInMin / 2)
        m_timelineModel.addNextMinute();
}

QString AudioPlayerController::pointerPositionToString(qint64 position)
{
    QString millis = QString::number((position / 10) % 100);
    if (millis.size() == 1) millis = "0" + millis;
    QString seconds = QString::number((position / 1000) % 60);
    if (seconds.size() == 1) seconds = "0" + seconds;
    QString mins = QString::number(position / 60000);
    if (mins.size() == 1) mins = "0" + mins;
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

AudioAmplitudeModel *AudioPlayerController::audioAmplitudeModel() { return &m_audioAmplitudeModel; }
TimelineModel *AudioPlayerController::timelineModel() { return &m_timelineModel; }
bool AudioPlayerController::isPlaybackAvailable() const { return m_isPlaybackAvailable; }

void AudioPlayerController::setIsPlaybackAvailable(bool newIsPlaybackAvailable)
{
    if (m_isPlaybackAvailable == newIsPlaybackAvailable) return;
    m_isPlaybackAvailable = newIsPlaybackAvailable;
    emit isPlaybackAvailableChanged();
}

void AudioPlayerController::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia)
        setIsPlaybackAvailable(true);
    else if (status == QMediaPlayer::InvalidMedia)
        setIsPlaybackAvailable(false);
    else if (status == QMediaPlayer::EndOfMedia) {
        m_isPlaying = false;
        emit isPlayingChanged();
    }
}

void AudioPlayerController::onProbeRecieved(QAudioBuffer buffer)
{
    if (m_isPlaying) {
        qreal amplitude = AudioBufferExtension::calculateAmplitude(buffer);
        int currentElementIndex = (qreal)m_player.position() / s_millisInSec * s_measurementsPerSec;
        m_audioAmplitudeModel.setValueForElement(amplitude, currentElementIndex);
    }
}

void AudioPlayerController::onDurationChanged(qint64 duration)
{
    if (!m_isDurationCalculated && m_isPlayerPage && duration > 0) {
        m_timelineModel.fillModel(duration / s_millisInMin);
        m_audioAmplitudeModel.fillModel((qreal)duration / s_millisInSec * s_measurementsPerSec + 1);
        m_isDurationCalculated = true;
    }
}
