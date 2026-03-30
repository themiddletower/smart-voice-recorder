#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QAudioRecorder>
#include <QUrl>
#include <QDebug>
#include <QAudioBuffer>
#include <QAudioProbe>

class AudioRecorder : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorder(QObject *parent = nullptr);
    void start();
    void stop();
    void pause();
    qreal getCurrentLevel();
    void setRecordSettings(QString codec, QString container);

private:
    QAudioRecorder m_audioRecorder;
    QAudioProbe m_probe;
    qreal m_currentLevel;
    bool m_isNewRecord;
    QString generateFileName();

signals:
    void durationChanged(qint64 duration);
    void error(QString error);
    void recordStarted();
    void recordPaused();
    void recordStopped();
    void recorderPrepared();
    void audiofilePathChanged(QString path);

public slots:
    void onRecordError(QMediaRecorder::Error errorMsg);
    void onProbeRecieved(QAudioBuffer buffer);
    void onRecorderStatusChanged(QMediaRecorder::Status status);
};

#endif
