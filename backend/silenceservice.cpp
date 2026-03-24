// SPDX-FileCopyrightText: 2024 Open Mobile Platform LLC community@omp.ru
// SPDX-License-Identifier: BSD-3-Clause

#include "silenceservice.h"
#include "silenceremover_qt.h" // ИЗМЕНЕНО: Подключаем новый класс

#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QFile>

SilenceService::SilenceService(QObject *parent) : QObject(parent) {}

QString SilenceService::makeOutputWavPath(const QString &inputPath) const
{
    QFileInfo fi(inputPath);

    const QString outDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QDir().mkpath(outDir);

    const QString base = fi.completeBaseName().isEmpty() ? "audio" : fi.completeBaseName();
    const QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    return QDir(outDir).absoluteFilePath(QString("%1_cut_%2.wav").arg(base, ts));
}

QVariantMap SilenceService::removeSilence(const QString &inputPath,
                                          const QVariantList &annotations,
                                          int silenceType)
{
    QVariantMap result;

    if (inputPath.isEmpty()) {
        result["error"] = "inputPath is empty";
        return result;
    }

    const QString outWav = makeOutputWavPath(inputPath);

    // ИЗМЕНЕНО: Используем новый класс SilenceRemoverQt
    SilenceRemoverQt remover;
    SilenceRemoveResult r = remover.removeSilenceToWav(inputPath, annotations, silenceType, outWav);

    if (!r.error.isEmpty()) {
        result["error"] = r.error;
        return result;
    }

    if (r.outputPath.isEmpty() || !QFile::exists(r.outputPath)) {
        result["error"] = "Output file missing: " + r.outputPath;
        return result;
    }

    result["outputPath"] = r.outputPath;
    result["annotations"] = r.newAnnotations;
    return result;
}
