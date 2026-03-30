#include "audiorecordercontroller.h"

AudioRecorderController::AudioRecorderController(QObject *parent) : QObject(parent)
{
    connect(&audioRecorder, &AudioRecorder::durationChanged, this,
            &AudioRecorderController::audioDurationChanged);
    connect(&audioRecorder, &AudioRecorder::error, this,
            &AudioRecorderController::recordErrorOccured);
    connect(&audioRecorder, &AudioRecorder::recordStarted, this,
            &AudioRecorderController::recordStarted);
    connect(&audioRecorder, &AudioRecorder::recordPaused, this,
            &AudioRecorderController::recordPaused);
    connect(&audioRecorder, &AudioRecorder::recordStopped, this,
            &AudioRecorderController::recordStopped);
    connect(&audioRecorder, &AudioRecorder::recorderPrepared, this,
            &AudioRecorderController::recorderPrepared);
    connect(&audioRecorder, &AudioRecorder::audiofilePathChanged, this,
            &AudioRecorderController::audiofilePathChanged);
}

void AudioRecorderController::startRecord()
{
    if (isRecordStarted) {
        isRecordStarted = false;
        audioRecorder.pause();
    } else {
        isRecordStarted = true;
        audioRecorder.start();
    }
}

void AudioRecorderController::stopRecord()
{
    isRecordStarted = false;
    audioRecorder.stop();
}

void AudioRecorderController::setRecordSettings(QString codec, QString container)
{
    audioRecorder.setRecordSettings(codec, container);
}

void AudioRecorderController::setDefaultRecordSettings()
{
    QAudioRecorder recorder;
    QStringList containers = recorder.supportedContainers();
    QStringList codecs = recorder.supportedAudioCodecs();

    qDebug() << "Available containers:" << containers;
    qDebug() << "Available codecs:" << codecs;

    if (containers.isEmpty() || codecs.isEmpty()) {
        qDebug() << "ERROR: No supported containers or codecs found";
        emit recordErrorOccured("No supported containers or codecs found");
        return;
    }

    // Предпочитаем wav + PCM — самый совместимый вариант
    QString codec = "audio/PCM";
    QString container = "wav";

    if (!codecs.contains(codec))
        codec = codecs.first();
    if (!containers.contains(container))
        container = containers.first();

    qDebug() << "Using codec:" << codec << "container:" << container;
    audioRecorder.setRecordSettings(codec, container);

    // Принудительно сообщаем что готовы
    emit recorderPrepared();
}

qreal AudioRecorderController::getAudioLevel()
{
    return audioRecorder.getCurrentLevel();
}
