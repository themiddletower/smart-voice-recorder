#include <QDateTime>
#include <QStandardPaths>
#include <qendian.h>

#include "audiorecorder.h"
#include "audiobufferextension.h"

AudioRecorder::AudioRecorder(QObject *parent) : QObject(parent), m_isNewRecord(true), m_currentLevel(0.0)
{
    m_probe.setSource(&m_audioRecorder);
    connect(&m_probe, &QAudioProbe::audioBufferProbed, this, &AudioRecorder::onProbeRecieved);
    connect(&m_audioRecorder,
            static_cast<void (QMediaRecorder::*)(QMediaRecorder::Error)>(&QMediaRecorder::error),
            this, &AudioRecorder::onRecordError);
    connect(&m_audioRecorder, &QAudioRecorder::statusChanged, this,
            &AudioRecorder::onRecorderStatusChanged);
}

QString AudioRecorder::generateFileName()
{
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("yyyyMMdd_hhmmss");
    return "record_" + formattedTime;
}

void AudioRecorder::start()
{
    if (m_isNewRecord) {
        QString audiofilePath =
            QString("%1/%2.%3")
                .arg(QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
                     generateFileName(), m_audioRecorder.containerFormat());
        m_audioRecorder.setOutputLocation(QUrl(audiofilePath));
        emit audiofilePathChanged(audiofilePath);
    }
    m_audioRecorder.record();
    m_isNewRecord = false;
}

void AudioRecorder::stop()
{
    m_audioRecorder.stop();
    m_isNewRecord = true;
}

void AudioRecorder::pause()
{
    m_audioRecorder.pause();
}

void AudioRecorder::setRecordSettings(QString codec, QString container)
{
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec(codec);
    audioSettings.setQuality(QMultimedia::HighQuality);
    m_audioRecorder.setEncodingSettings(audioSettings);
    m_audioRecorder.setContainerFormat(container);
}

qreal AudioRecorder::getCurrentLevel()
{
    return m_currentLevel;
}

void AudioRecorder::onRecordError(QMediaRecorder::Error errorMsg)
{
    Q_UNUSED(errorMsg)
    emit error(m_audioRecorder.errorString());
}

void AudioRecorder::onProbeRecieved(QAudioBuffer buffer)
{
    m_currentLevel = AudioBufferExtension::calculateAmplitude(buffer);
    emit durationChanged(m_audioRecorder.duration());
}

void AudioRecorder::onRecorderStatusChanged(QMediaRecorder::Status status)
{
    if (status == QAudioRecorder::LoadedStatus)
        emit recorderPrepared();
    else if (status == QAudioRecorder::RecordingStatus)
        emit recordStarted();
    else if (status == QAudioRecorder::PausedStatus)
        emit recordPaused();
    else if (status == QAudioRecorder::FinalizingStatus)
        emit recordStopped();
}
