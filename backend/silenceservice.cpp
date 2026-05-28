#include "silenceservice.h"
#include "silenceremover_qt.h"

#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QCryptographicHash>

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QUrl>

SilenceService::SilenceService(QObject *parent) : QObject(parent) {}

QString SilenceService::makeTempWavPath() const
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);

    const QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    return QDir(cacheDir).absoluteFilePath(QString("temp_edit_%1.wav").arg(ts));
}

QString makeOutputWavPath(const QString &inputPath)
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

    const QString outWav = makeTempWavPath();

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

QString SilenceService::getMusicPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
}


bool SilenceService::finalizeSave(const QString &currentTempPath, const QString &newFileName)
{
    QString src = currentTempPath;
    if (src.startsWith("file://")) src = QUrl(src).toLocalFile();

    QString musicDir = getMusicPath();
    QDir().mkpath(musicDir);

    QString dest = musicDir + "/" + newFileName;
    if (!dest.endsWith(".wav")) dest += ".wav";

    qDebug() << "[C++] Попытка сохранения аудио:";
    qDebug() << "[C++] Из:" << src;
    qDebug() << "[C++] В:" << dest;

    if (QFile::exists(dest)) {
        qDebug() << "[C++] Файл уже существует, удаляю...";
        QFile::remove(dest);
    }

    bool ok = QFile::copy(src, dest);
    qDebug() << "[C++] Результат копирования:" << ok;
    return ok;
}
QString SilenceService::getFileHash(const QString &filePath) {
    QString path = filePath;
    if (path.startsWith("file://")) path = QUrl(path).toLocalFile();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return "";

    QFileInfo fileInfo(path);
    qint64 fileSize = fileInfo.size();

    QByteArray data = file.read(102400);

    if (fileSize > 102400 + 4096) {
        file.seek(fileSize - 4096);
        data.append(file.readAll());
    }

    file.close();

    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

    return QString::number(fileSize) + "_" + QString(hash);
}

bool SilenceService::saveMetadata(const QString &audioPath, const QVariantList &annotations, const QVariantList &voiceLabels) {
    QString cleanPath = audioPath;
    if (cleanPath.startsWith("file://")) cleanPath = QUrl(cleanPath).toLocalFile();

    QString jsonPath = cleanPath + ".json";
    qDebug() << "[C++] Сохранение метаданных в:" << jsonPath;

    QJsonObject root;
    root["annotations"] = QJsonArray::fromVariantList(annotations);
    root["voiceLabels"] = QJsonArray::fromVariantList(voiceLabels);

    QFile file(jsonPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "[C++] ОШИБКА: Не удалось открыть файл для записи!";
        return false;
    }

    file.write(QJsonDocument(root).toJson());
    file.close();
    qDebug() << "[C++] Метаданные успешно записаны.";
    return true;
}

QVariantMap SilenceService::loadMetadata(const QString &audioPath) {
    QString path = audioPath;
    if (path.startsWith("file://")) path = QUrl(path).toLocalFile();

    QString jsonPath = path + ".json";
    QVariantMap result;

    QFile file(jsonPath);
    if (!file.exists()) {
        qDebug() << "[C++] JSON не найден по пути:" << jsonPath;
        return result;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[C++] Ошибка открытия JSON:" << file.errorString();
        return result;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        result["annotations"] = obj["annotations"].toArray().toVariantList();
        result["voiceLabels"] = obj["voiceLabels"].toArray().toVariantList();
        qDebug() << "[C++] Успешно загружено аннотаций:" << result["annotations"].toList().size();
    }

    file.close();
    return result;
}

QString SilenceService::getAppDataPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

Q_INVOKABLE QVariantMap SilenceService::loadMetadataFromFile(const QString &jsonPath) {
    QVariantMap result;
    QFile file(jsonPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return result;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isObject()) {
        result["annotations"] = doc.object()["annotations"].toArray().toVariantList();
        result["voiceLabels"] = doc.object()["voiceLabels"].toArray().toVariantList();
    }
    return result;
}

