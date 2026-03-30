#ifndef SILENCEREMOVER_H
#define SILENCEREMOVER_H

#include <QVariantList>
#include <QString>

struct SilenceRemoveResultFfmpeg
{
    QString outputPath;
    QVariantList newAnnotations;
    QString error;
};

class SilenceRemover
{
public:
    SilenceRemover() = default;
    struct Interval { qint64 t1; qint64 t2; };

    SilenceRemoveResultFfmpeg removeSilence(const QString &inputPath,
                                            const QVariantList &annotations,
                                            int silenceType,
                                            const QString &outputPath);

private:
    static QList<Interval> collectAndMergeSilences(const QVariantList &annotations, int silenceType);
    static qint64 removedBefore(const QList<Interval> &silences, qint64 t);

    static QVariantList recalcAnnotations(const QVariantList &annotations,
                                          const QList<Interval> &silences,
                                          int silenceType);

    static qint64 probeDurationMs(const QString &inputPath, QString *error);

    static QString buildFfmpegFilterKeepIntervals(const QList<Interval> &silences,
                                                  qint64 durationMs,
                                                  QString *error);

    static bool runProcess(const QString &program,
                           const QStringList &args,
                           int *exitCode,
                           QString *stdOut,
                           QString *stdErr);

    static bool ensureFfmpegAvailable(QString *error);
};

#endif // SILENCEREMOVER_H
