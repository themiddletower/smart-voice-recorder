// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include <QDateTime>
#include <QStandardPaths>
#include <qendian.h>

#include "audiorecorderPlayer.h"
#include "audiobufferextensionPlayer.h"

AudioRecorder::AudioRecorder(QObject *parent) : QObject(parent), m_isNewRecord(true)
{
    m_probe.setSource(&m_audioRecorder);
    connect(&m_probe, &QAudioProbe::audioBufferProbed, this, &AudioRecorder::onProbeRecieved);

    connect(&m_audioRecorder,
            static_cast<void (QMediaRecorder::*)(QMediaRecorder::Error)>(&QMediaRecorder::error),
            this, &AudioRecorder::onRecordError);
    connect(&m_audioRecorder, &QAudioRecorder::statusChanged, this,
            &AudioRecorder::onRecorderStatusChanged);
}

/*!
 * \brief Generates a result file name taking into the current date.
 */
QString AudioRecorder::generateFileName()
{
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("yyyyMMdd_hhmmss");
    return "record_" + formattedTime;
}

/*!
 * \brief Starts sound recording.
 */
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

/*!
 * \brief Stops sound recording.
 */
void AudioRecorder::stop()
{
    m_audioRecorder.stop();
    m_isNewRecord = true;
}

/*!
 * \brief Pauses sound recording.
 */
void AudioRecorder::pause()
{
    m_audioRecorder.pause();
}

/*!
 * \brief Sets record settings.
 * \param codec Record codec.
 * \param container Record container.
 */
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

/*!
 * \brief Emits the error signal with the error message when any record error is occurred.
 */
void AudioRecorder::onRecordError(QMediaRecorder::Error errorMsg)
{
    qDebug() << "Error:" << m_audioRecorder.errorString();
    emit error(m_audioRecorder.errorString());
}

/*!
 * \brief Handles the new probe recieving. Processes data and calculate current audio level.
 * \param buffer Audio data from audio recorder.
 */
void AudioRecorder::onProbeRecieved(QAudioBuffer buffer)
{
    m_currentLevel = AudioBufferExtension::calculateAmplitude(buffer);
    emit durationChanged(m_audioRecorder.duration());
}

/*!
 * \brief Handles the recorder status changes.
 * \param status New status.
 */
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
