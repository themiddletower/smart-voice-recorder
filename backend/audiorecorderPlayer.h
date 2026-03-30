#ifndef AUDIORECORDERPLAYER_H
#define AUDIORECORDERPLAYER_H

#include <QObject>
#include <QAudioRecorder>
#include <QUrl>
#include <QDebug>
#include <QAudioBuffer>
#include <QAudioProbe>

class AudioRecorderRedact : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorderRedact(QObject *parent = nullptr);
    void start();
    void stop();
    void pause();

    qreal getCurrentLevel();
    void setRecordSettings(QString codec, QString container);

private:
    QAudioRecorder m_audioRecorder;
    QAudioProbe m_probe;
    bool m_isNewRecord;       // сначала bool
    qreal m_currentLevel;     // потом qreal

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

#endif // AUDIORECORDERPLAYER_H
