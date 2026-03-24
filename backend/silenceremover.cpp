// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#include "silenceremover.h"

#include <QProcess>
#include <QVariantMap>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QtMath>

static bool intervalLess(const SilenceRemover::Interval &a, const SilenceRemover::Interval &b)
{
    return a.t1 < b.t1;
}

bool SilenceRemover::runProcess(const QString &program,
                                const QStringList &args,
                                int *exitCode,
                                QString *stdOut,
                                QString *stdErr)
{
    QProcess p;
    p.start(program, args);
    if (!p.waitForStarted(5000)) {
        if (exitCode) *exitCode = -1;
        if (stdErr) *stdErr = program + " failed to start";
        return false;
    }
    p.waitForFinished(-1);

    if (exitCode) *exitCode = p.exitCode();
    if (stdOut) *stdOut = QString::fromUtf8(p.readAllStandardOutput());
    if (stdErr) *stdErr = QString::fromUtf8(p.readAllStandardError());

    return p.exitStatus() == QProcess::NormalExit;
}

bool SilenceRemover::ensureFfmpegAvailable(QString *error)
{
    int code = 0;
    QString out, err;

    // ffmpeg
    if (!runProcess("ffmpeg", { "-version" }, &code, &out, &err) || code != 0) {
        if (error) *error = "ffmpeg is not available: " + (err.isEmpty() ? out : err);
        return false;
    }

    // ffprobe
    if (!runProcess("ffprobe", { "-version" }, &code, &out, &err) || code != 0) {
        if (error) *error = "ffprobe is not available: " + (err.isEmpty() ? out : err);
        return false;
    }

    return true;
}

QList<SilenceRemover::Interval>
SilenceRemover::collectAndMergeSilences(const QVariantList &annotations, int silenceType)
{
    QList<Interval> silences;
    for (const QVariant &v : annotations) {
        const QVariantMap m = v.toMap();
        if (m.value("type").toInt() != silenceType)
            continue;

        Interval in{ m.value("t1").toLongLong(), m.value("t2").toLongLong() };
        if (in.t2 > in.t1)
            silences.append(in);
    }

    std::sort(silences.begin(), silences.end(), intervalLess);

    QList<Interval> merged;
    for (const auto &s : silences) {
        if (merged.isEmpty() || s.t1 > merged.last().t2) {
            merged.append(s);
        } else {
            merged.last().t2 = qMax(merged.last().t2, s.t2);
        }
    }
    return merged;
}

qint64 SilenceRemover::removedBefore(const QList<Interval> &silences, qint64 t)
{
    qint64 removed = 0;
    for (const auto &s : silences) {
        if (t >= s.t2) {
            removed += (s.t2 - s.t1);
        } else if (t <= s.t1) {
            break;
        } else {
            removed += (t - s.t1);
            break;
        }
    }
    return removed;
}

QVariantList SilenceRemover::recalcAnnotations(const QVariantList &annotations,
                                               const QList<Interval> &silences,
                                               int silenceType)
{
    QVariantList out;
    for (const QVariant &v : annotations) {
        const QVariantMap m = v.toMap();
        const int type = m.value("type").toInt();
        if (type == silenceType)
            continue;

        const qint64 t1 = m.value("t1").toLongLong();
        const qint64 t2 = m.value("t2").toLongLong();

        const qint64 nt1 = t1 - removedBefore(silences, t1);
        const qint64 nt2 = t2 - removedBefore(silences, t2);

        QVariantMap nm;
        nm["t1"] = nt1;
        nm["t2"] = nt2;
        nm["type"] = type;
        out.append(nm);
    }
    return out;
}

qint64 SilenceRemover::probeDurationMs(const QString &inputPath, QString *error)
{
    int code = 0;
    QString out, err;

    // ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 input
    const QStringList args{
        "-v", "error",
        "-show_entries", "format=duration",
        "-of", "default=noprint_wrappers=1:nokey=1",
        inputPath
    };

    if (!runProcess("ffprobe", args, &code, &out, &err) || code != 0) {
        if (error) *error = "ffprobe failed: " + (err.isEmpty() ? out : err);
        return 0;
    }

    const QString s = out.trimmed();
    bool ok = false;
    const double sec = s.toDouble(&ok);
    if (!ok || sec <= 0) {
        if (error) *error = "ffprobe returned invalid duration: '" + s + "'";
        return 0;
    }

    return qint64(qRound64(sec * 1000.0));
}

QString SilenceRemover::buildFfmpegFilterKeepIntervals(const QList<Interval> &silences,
                                                       qint64 durationMs,
                                                       QString *error)
{
    if (durationMs <= 0) {
        if (error) *error = "Invalid duration";
        return {};
    }

    // Build keep intervals [0..sil1.t1], [sil1.t2..sil2.t1], ..., [last.t2..duration]
    QList<Interval> keeps;
    qint64 cur = 0;
    for (const auto &s : silences) {
        if (s.t1 > cur)
            keeps.append({cur, s.t1});
        cur = qMax(cur, s.t2);
    }
    if (durationMs > cur)
        keeps.append({cur, durationMs});

    if (keeps.isEmpty()) {
        if (error) *error = "All audio is silence (nothing to keep)";
        return {};
    }

    // Build filter_complex with atrim + concat
    QString filter;
    QStringList labels;

    for (int i = 0; i < keeps.size(); ++i) {
        const double start = keeps[i].t1 / 1000.0;
        const double end = keeps[i].t2 / 1000.0;
        const QString lab = QString("a%1").arg(i);
        labels << QString("[%1]").arg(lab);

        filter += QString("[0:a]atrim=start=%1:end=%2,asetpts=PTS-STARTPTS[%3];")
                      .arg(start, 0, 'f', 6)
                      .arg(end,   0, 'f', 6)
                      .arg(lab);
    }

    filter += QString("%1concat=n=%2:v=0:a=1[out]")
                  .arg(labels.join(""))
                  .arg(keeps.size());

    return filter;
}

SilenceRemoveResult SilenceRemover::removeSilence(const QString &inputPath,
                                                  const QVariantList &annotations,
                                                  int silenceType,
                                                  const QString &outputPath)
{
    SilenceRemoveResult res;

    if (inputPath.isEmpty()) {
        res.error = "inputPath is empty";
        return res;
    }
    if (outputPath.isEmpty()) {
        res.error = "outputPath is empty";
        return res;
    }
    if (!QFile::exists(inputPath)) {
        res.error = "Input file not found: " + inputPath;
        return res;
    }

    QString ffErr;
    if (!ensureFfmpegAvailable(&ffErr)) {
        res.error = ffErr;
        return res;
    }

    const QList<Interval> silences = collectAndMergeSilences(annotations, silenceType);
    res.newAnnotations = recalcAnnotations(annotations, silences, silenceType);

    QString durErr;
    const qint64 durMs = probeDurationMs(inputPath, &durErr);
    if (durMs <= 0) {
        res.error = durErr.isEmpty() ? "Failed to get duration" : durErr;
        return res;
    }

    QString filterErr;
    const QString filter = buildFfmpegFilterKeepIntervals(silences, durMs, &filterErr);
    if (filter.isEmpty()) {
        res.error = filterErr.isEmpty() ? "Failed to build ffmpeg filter" : filterErr;
        return res;
    }

    // Ensure output dir exists
    QFileInfo ofi(outputPath);
    QDir().mkpath(ofi.absolutePath());

    // Run ffmpeg
    int code = 0;
    QString out, err;
    const QStringList args{
        "-y",
        "-i", inputPath,
        "-filter_complex", filter,
        "-map", "[out]",
        outputPath
    };

    if (!runProcess("ffmpeg", args, &code, &out, &err) || code != 0) {
        res.error = "ffmpeg failed (exitCode=" + QString::number(code) + "): " + (err.isEmpty() ? out : err);
        return res;
    }

    if (!QFile::exists(outputPath)) {
        res.error = "Output file was not created: " + outputPath;
        return res;
    }

    res.outputPath = outputPath;
    return res;
}

