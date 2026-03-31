// src/audiosegmenter.h
//
// Главный класс фичи.
// Принимает путь к WAV-файлу → возвращает список сегментов.
//
// Использование из QML:
//   segmenter.processFile("/path/to/audio.wav")
//   → segmentModel заполняется сегментами
//   → segmenter.status меняется на "done"

#ifndef AUDIOSEGMENTER_H
#define AUDIOSEGMENTER_H

#include <QObject>
#include <QString>
#include <QVector>

#include "audiosegment.h"
#include "vadengine.h"
#include "wavreader.h"

class SegmentListModel;

class AudioSegmenter : public QObject
{
    Q_OBJECT

    // Свойства для QML
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(qint64 fileDurationMs READ fileDurationMs NOTIFY fileInfoChanged)
    Q_PROPERTY(int fileSampleRate READ fileSampleRate NOTIFY fileInfoChanged)

public:
    explicit AudioSegmenter(QObject* parent = nullptr);

    // Привязываем модель, куда будут добавляться сегменты
    void setSegmentModel(SegmentListModel* model);

    // Getters для QML
    QString status() const;
    int progress() const;          // 0–100
    QString errorMessage() const;
    qint64 fileDurationMs() const;
    int fileSampleRate() const;

    // ─── Настройки VAD ───
    Q_INVOKABLE void setVadMode(int mode);
    Q_INVOKABLE void setSilenceThresholdDb(float db);
    Q_INVOKABLE void setHangoverMs(int ms);

public slots:
    // Запустить сегментацию файла
    void processFile(const QString& filePath);

signals:
    void statusChanged();
    void progressChanged();
    void errorChanged();
    void fileInfoChanged();
    void finished();

private:
    // Закрыть текущий сегмент и добавить в модель
    void closeSegment(qint64 endMs);

    SegmentListModel* m_model;
    VadEngine         m_vad;

    QString m_status;       // "idle" / "processing" / "done" / "error"
    int     m_progress;
    QString m_errorMessage;
    qint64  m_fileDurationMs;
    int     m_fileSampleRate;

    // Состояние текущего сегмента (при обработке)
    AudioSegment::Type m_curType;
    qint64  m_curStartMs;
    float   m_curLevelSum;
    float   m_curPeak;
    int     m_curFrameCount;
};

#endif // AUDIOSEGMENTER_H
