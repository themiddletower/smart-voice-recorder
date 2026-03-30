#ifndef AUDIORECORDERCONTROLLER_H
#define AUDIORECORDERCONTROLLER_H

#include <QObject>
#include "audiorecorder.h"

class AudioRecorderController : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorderController(QObject *parent = nullptr);
    Q_INVOKABLE void startRecord();
    Q_INVOKABLE void stopRecord();
    Q_INVOKABLE void setRecordSettings(QString codec, QString container);
    Q_INVOKABLE void setDefaultRecordSettings();
    Q_INVOKABLE qreal getAudioLevel();

private:
    AudioRecorder audioRecorder;
    bool isRecordStarted = false;

signals:
    void audioDurationChanged(quint64 duration);
    void recordStarted();
    void recordPaused();
    void recordStopped();
    void recorderPrepared();
    void recordErrorOccured(QString error);
    void audiofilePathChanged(QString path);
};

#endif
