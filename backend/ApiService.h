#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QStringList>

class ApiService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList fileListModel READ fileListModel NOTIFY fileListChanged)

public:
    explicit ApiService(QObject *parent = nullptr);

    Q_INVOKABLE void registerUser(const QString& username, const QString& password);
    Q_INVOKABLE void login(const QString& username, const QString& password);
    Q_INVOKABLE void sendDataToServer(const QString& filePath, const QString& metadataJson);
    Q_INVOKABLE void downloadMetadata(const QString& fileName);

    Q_INVOKABLE void refreshFileList();
    Q_INVOKABLE void deleteFile(const QString& fileName);
    Q_INVOKABLE void downloadFile(const QString& fileName);
    Q_INVOKABLE void renameFile(const QString& oldName, const QString& newName);
    Q_INVOKABLE QString getSavedUsername() const;
    Q_INVOKABLE QString getSavedPassword() const;

    QStringList fileListModel() const { return m_fileList; }

signals:
    void fileListChanged();
    void fileDownloaded(const QString& filePath);

    // сигналы для точного контроля оверлея загрузки в QML
    void loginSuccess();
    void loginError(const QString& errorText);
    void registerSuccess();
    void registerError(const QString& errorText);
    void uploadSuccess();
    void apiError(const QString& errorText);

private:
    QNetworkAccessManager *manager;
    QString authToken;
    QString baseUrl = "https://friskily-resounding-barnacle.cloudpub.ru:443";
    QStringList m_fileList;
};

#endif // APISERVICE_H
