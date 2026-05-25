#include "ApiService.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

QSettings settings;

ApiService::ApiService(QObject *parent) : QObject(parent) {
    authToken = settings.value("auth/token").toString();
    manager = new QNetworkAccessManager(this);
}

QString ApiService::getSavedUsername() const {
    return settings.value("auth/username").toString();
}

QString ApiService::getSavedPassword() const {
    return settings.value("auth/password").toString();
}

void ApiService::sendDataToServer(const QString& filePath,
                             const QString& metadataJson)
{
    // ------------------------------------------------
    // Проверка metadata
    // ------------------------------------------------

    if (metadataJson.isEmpty()) {
        qDebug() << "[ApiService] metadata пустой";
        return;
    }

    // ------------------------------------------------
    // Проверка авторизации
    // ------------------------------------------------

    if (authToken.isEmpty()) {
        qDebug() << "[ApiService] Нет токена";
        return;
    }

    // ------------------------------------------------
    // Request
    // ------------------------------------------------

    QUrl url(baseUrl + "/api/records");

    QNetworkRequest request(url);

    request.setRawHeader(
        "Authorization",
        ("Bearer " + authToken).toUtf8()
        );

    request.setRawHeader(
        "Accept",
        "application/json"
        );

    // ------------------------------------------------
    // Multipart
    // ------------------------------------------------

    QHttpMultiPart *multiPart =
        new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // =====================================================
    // METADATA
    // =====================================================

    QHttpPart metadataPart;

    metadataPart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant("form-data; name=\"metadata\"")
        );

    metadataPart.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QVariant("application/json")
        );

    metadataPart.setBody(
        metadataJson.toUtf8()
        );

    // =====================================================
    // AUDIO
    // =====================================================

    QFile *file = new QFile(filePath);

    if (!file->exists() ||
        !file->open(QIODevice::ReadOnly))
    {
        qDebug() << "[ApiService] Не удалось открыть файл:"
                 << filePath;

        delete multiPart;
        return;
    }

    QString fileName =
        QFileInfo(filePath).fileName();

    QHttpPart audioPart;

    audioPart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(
            QString(
                "form-data; name=\"file\"; filename=\"%1\""
                ).arg(fileName)
            )
        );

    audioPart.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QVariant("audio/wav")
        );

    audioPart.setBodyDevice(file);

    file->setParent(multiPart);

    // =====================================================
    // APPEND
    // =====================================================

    multiPart->append(metadataPart);
    multiPart->append(audioPart);

    // =====================================================
    // SEND
    // =====================================================

    QNetworkReply *reply =
        manager->post(request, multiPart);

    multiPart->setParent(reply);

    connect(reply,
            &QNetworkReply::finished,
            [reply]()
            {
                if (reply->error() ==
                    QNetworkReply::NoError)
                {
                    qDebug() << "[ApiService] Upload success";
                    qDebug() << reply->readAll();
                }
                else
                {
                    qDebug() << "[ApiService] Upload error";

                    qDebug() << "HTTP:"
                             << reply->attribute(
                                         QNetworkRequest::HttpStatusCodeAttribute
                                         ).toInt();

                    qDebug() << "ERROR:"
                             << reply->errorString();

                    qDebug() << "SERVER:"
                             << reply->readAll();
                }

                reply->deleteLater();
            });
}

void ApiService::downloadMetadata(const QString& fileName)
{
    if (authToken.isEmpty())
        return;

    QUrl url(
        baseUrl +
        "/api/records/" +
        fileName +
        "/metadata"
        );

    QNetworkRequest request(url);

    request.setRawHeader(
        "Authorization",
        ("Bearer " + authToken).toUtf8()
        );

    QNetworkReply* reply =
        manager->get(request);

    connect(reply,
            &QNetworkReply::finished,
            this,
            [reply, fileName]()
            {
                reply->deleteLater();

                if (reply->error() !=
                    QNetworkReply::NoError)
                {
                    qDebug() << "[ApiService] Metadata download error:"
                             << reply->errorString();

                    return;
                }

                QByteArray data = reply->readAll();

                QString dir =
                    QStandardPaths::writableLocation(
                        QStandardPaths::MusicLocation
                        );

                QDir().mkpath(dir);

                QString jsonPath =
                    dir + "/" + fileName + ".json";

                QFile file(jsonPath);

                if (!file.open(QIODevice::WriteOnly))
                {
                    qDebug() << "[ApiService] Cannot save metadata:"
                             << jsonPath;

                    return;
                }

                file.write(data);
                file.close();

                qDebug() << "[ApiService] Metadata saved:"
                         << jsonPath;
            });
}

void ApiService::login(const QString& username, const QString& password) {
    QUrl url(baseUrl + "/api/login");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    QNetworkReply* reply = manager->post(request, QJsonDocument(json).toJson());

    // В ApiService.cpp в методе login
    connect(reply, &QNetworkReply::finished, [this, reply, username, password]() {
        QByteArray response = reply->readAll();
        qDebug() << "LOGIN RESPONSE:" << response;

        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject() && doc.object().contains("access_token")) {
            // Убедитесь, что записываете в переменную класса
            this->authToken = doc.object()["access_token"].toString();

            settings.setValue("auth/token", authToken);
            settings.setValue("auth/username", username);
            settings.setValue("auth/password", password);
            qDebug() << "TOKEN SUCCESSFULLY SAVED:" << this->authToken;
        } else {
            qDebug() << "FAILED TO GET TOKEN FROM RESPONSE";
        }
        reply->deleteLater();
    });
}

void ApiService::registerUser(const QString& username, const QString& password) {
    QUrl url(baseUrl + "/api/register");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    QNetworkReply* reply = manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, [reply]() {
        qDebug() << "REGISTER RESPONSE:" << reply->readAll();
        reply->deleteLater();
    });
}

void ApiService::refreshFileList() {
    if (authToken.isEmpty()) return;

    QUrl url(baseUrl + "/api/records");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + authToken.toUtf8());

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray filesArray = doc.object()["files"].toArray();

            m_fileList.clear();
            for (const QJsonValue & value : filesArray) {
                m_fileList << value.toString();
            }
            emit fileListChanged(); // Уведомляем QML об обновлении
            qDebug() << "[ApiService] Список файлов обновлен:" << m_fileList;
        } else {
            qDebug() << "[ApiService] Ошибка получения списка:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void ApiService::deleteFile(const QString& fileName) {
    if (authToken.isEmpty()) return;

    QUrl url(baseUrl + "/api/records/" + fileName);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + authToken.toUtf8());

    QNetworkReply* reply = manager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "[ApiService] Файл успешно удален";
            refreshFileList(); // Сразу обновляем список после удаления
        } else {
            qDebug() << "[ApiService] Ошибка удаления:" << reply->readAll();
        }
        reply->deleteLater();
    });
}

void ApiService::downloadFile(const QString& fileName)
{
    if (authToken.isEmpty())
        return;

    QUrl url(baseUrl + "/api/records/" + fileName);

    QNetworkRequest request(url);

    request.setRawHeader(
        "Authorization",
        ("Bearer " + authToken).toUtf8()
        );

    QNetworkReply* reply =
        manager->get(request);

    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, fileName]()
            {
                reply->deleteLater();

                if (reply->error() !=
                    QNetworkReply::NoError)
                {
                    qDebug() << "[ApiService] Download error:"
                             << reply->errorString();

                    return;
                }

                QByteArray data = reply->readAll();

                qDebug() << "[ApiService] File received:"
                         << fileName
                         << "size:"
                         << data.size();

                QString dir =
                    QStandardPaths::writableLocation(
                        QStandardPaths::MusicLocation
                        );

                QDir().mkpath(dir);

                QString filePath =
                    dir + "/" + fileName;

                QFile file(filePath);

                if (!file.open(QIODevice::WriteOnly))
                {
                    qDebug() << "[ApiService] Cannot write file:"
                             << filePath;

                    return;
                }

                file.write(data);
                file.close();

                qDebug() << "[ApiService] Saved:"
                         << filePath;

                // ==========================================
                // DOWNLOAD METADATA
                // ==========================================

                downloadMetadata(fileName);
            });
}

void ApiService::renameFile(const QString& oldName, const QString& newName)
{
    if (authToken.isEmpty()) return;

    QUrl url(baseUrl + "/api/records/" + oldName);
    QNetworkRequest request(url);

    request.setRawHeader("Authorization", "Bearer " + authToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["new_name"] = newName;

    QNetworkReply* reply = manager->put(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "[ApiService] Rename success";
            refreshFileList(); // обновляем список
        } else {
            qDebug() << "[ApiService] Rename error:" << reply->readAll();
        }
        reply->deleteLater();
    });
}


