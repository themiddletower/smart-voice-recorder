#include "silenceservice.h"
#include "silenceremover_qt.h"

#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QFile>

SilenceService::SilenceService(QObject *parent) : QObject(parent) {}

QString SilenceService::makeTempWavPath() const
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);

    const QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    return QDir(cacheDir).absoluteFilePath(QString("temp_edit_%1.wav").arg(ts));
}

QString makeOutputWavPath(const QString &inputPath) // старый метод оставляем для совместимости, но он больше не используется
{
    Q_UNUSED(inputPath);
    return QString();
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

    const QString outWav = makeTempWavPath(); // ← ВРЕМЕННЫЙ ФАЙЛ

    SilenceRemoverQt remover;
    SilenceRemoveResultQt r = remover.removeSilenceToWav(inputPath, annotations, silenceType, outWav);

    if (!r.error.isEmpty()) {
        result["error"] = r.error;
        return result;
    }

    if (!QFile::exists(r.outputPath)) {
        result["error"] = "Output file missing";
        return result;
    }

    result["outputPath"] = r.outputPath;
    result["annotations"] = r.newAnnotations;
    return result;
}

bool SilenceService::finalizeSave(const QString &currentTempPath, const QString &newFileName)
{
    if (currentTempPath.isEmpty() || newFileName.isEmpty() || !QFile::exists(currentTempPath))
        return false;

    QString finalName = newFileName.trimmed();
    if (!finalName.toLower().endsWith(".wav", Qt::CaseInsensitive))
        finalName += ".wav";

    const QString musicDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QDir().mkpath(musicDir);

    const QString finalPath = QDir(musicDir).absoluteFilePath(finalName);

    // Удаляем, если уже есть файл с таким именем
    if (QFile::exists(finalPath))
        QFile::remove(finalPath);

    return QFile::copy(currentTempPath, finalPath);
}
