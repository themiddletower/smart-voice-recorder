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

void ApiService::sendDataToServer(const QString& filePath, const QString& jsonAnnotations) {
    if (jsonAnnotations.isEmpty() || jsonAnnotations == "[]") {
        qDebug() << "[ApiService] Ошибка: нет аннотаций для отправки";
        return;
    }

    if (authToken.isEmpty()) {
        qDebug() << "[ApiService] Ошибка: пользователь не авторизован!";
        return;
    }

    QUrl url(baseUrl + "/api/records");
    QNetworkRequest request(url);

    QString headerData = "Bearer " + authToken;
    request.setRawHeader("Authorization", headerData.toUtf8());
    request.setRawHeader("Accept", "application/json");
    // Content-Type для multipart установит сам QNetworkAccessManager

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart jsonPart;
    jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"annotations\""));
    jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain; charset=utf-8"));
    jsonPart.setBody(jsonAnnotations.toUtf8());

    QFile *file = new QFile(filePath);
    if (!file->exists() || !file->open(QIODevice::ReadOnly)) {
        qDebug() << "[ApiService] Ошибка: файл не найден или недоступен:" << filePath;
        delete multiPart;
        return;
    }

    QHttpPart audioPart;
    QString fileName = filePath.split('/').last();
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/mpeg"));
    audioPart.setBodyDevice(file);

    file->setParent(multiPart); // Файл удалится вместе с multiPart

    multiPart->append(jsonPart);
    multiPart->append(audioPart);

    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply); // multiPart удалится вместе с reply

    connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "[ApiService] Успех:" << reply->readAll();
        } else {
            qDebug() << "[ApiService] Ошибка сервера:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "[ApiService] Текст ошибки:" << reply->errorString();
            qDebug() << "[ApiService] Ответ сервера:" << reply->readAll();
        }
        reply->deleteLater();
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

    request.setRawHeader("Authorization", "Bearer " + authToken.toUtf8());

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, fileName]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[ApiService] Download error:" << reply->errorString();
            return;
        }

        QByteArray data = reply->readAll();

        qDebug() << "[ApiService] File received:" << fileName
                 << "size:" << data.size();

        const QString dir =
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation);

        QDir().mkpath(dir);

        const QString filePath = dir + "/" + fileName;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "[ApiService] Cannot write file:" << filePath;
            return;
        }

        file.write(data);
        file.close();

        qDebug() << "[ApiService] Saved file to:" << filePath;
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


