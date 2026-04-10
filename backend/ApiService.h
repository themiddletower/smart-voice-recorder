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
    // Свойство для отображения списка файлов в QML
    Q_PROPERTY(QStringList fileListModel READ fileListModel NOTIFY fileListChanged)

public:
    explicit ApiService(QObject *parent = nullptr);

    Q_INVOKABLE void registerUser(const QString& username, const QString& password);
    Q_INVOKABLE void login(const QString& username, const QString& password);
    Q_INVOKABLE void sendDataToServer(const QString& filePath, const QString& jsonAnnotations);

    // НОВЫЕ МЕТОДЫ
    Q_INVOKABLE void refreshFileList();           // Получить список файлов
    Q_INVOKABLE void deleteFile(const QString& fileName); // Удалить файл
    Q_INVOKABLE void downloadFile(const QString& fileName); // Чтение/Загрузка
    Q_INVOKABLE void renameFile(const QString& oldName, const QString& newName);
    Q_INVOKABLE QString getSavedUsername() const;
    Q_INVOKABLE QString getSavedPassword() const;

    QStringList fileListModel() const { return m_fileList; }

signals:
    void fileListChanged();
    void fileDownloaded(const QString& filePath);

private:
    QNetworkAccessManager *manager;
    QString authToken;
    QString baseUrl = "https://friskily-resounding-barnacle.cloudpub.ru:443";
    QStringList m_fileList;
};

#endif // APISERVICE_H
