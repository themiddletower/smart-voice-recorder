import QtQuick 2.0
import Sailfish.Silica 1.0
import Aurora.Controls 1.0

Page {
    id: cloudPage
    property string currentLocalFile: ""
    property string currentAnnotations: "[]"

    // Предполагается, что ApiService зарегистрирован в C++ как "ApiService"
    // или доступен через контекстное свойство apiService

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader { title: "Облачное хранилище" }

            // --- Блок авторизации ---
            SectionHeader { text: "Авторизация" }

            TextField {
                id: userField
                width: parent.width - Theme.horizontalPageMargin * 2
                anchors.horizontalCenter: parent.horizontalCenter
                placeholderText: "Логин"
                label: "Пользователь"
                text: apiService.getSavedUsername()
            }

            PasswordField {
                id: passField
                width: parent.width - Theme.horizontalPageMargin * 2
                anchors.horizontalCenter: parent.horizontalCenter
                placeholderText: "Пароль"
                label: "Пароль"
                text: apiService.getSavedPassword()
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium
                Button {
                    text: "Вход"
                    onClicked: apiService.login(userField.text, passField.text)
                }
                Button {
                    text: "Регистрация"
                    onClicked: apiService.registerUser(userField.text, passField.text)
                }
            }

            // --- Блок управления файлами ---
            Separator {
                width: parent.width
                color: Theme.primaryColor
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Выгрузить текущий файл"
                enabled: cloudPage.currentLocalFile !== ""
                onClicked: apiService.sendDataToServer(cloudPage.currentLocalFile, cloudPage.currentAnnotations)
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Обновить список облака"
                onClicked: apiService.refreshFileList()
            }

            // --- Список файлов в облаке ---
            SectionHeader { text: "Файлы в облаке" }

            Repeater {
                model: apiService.fileListModel // Связано с m_fileList в C++
                delegate: ListItem {
                    width: cloudPage.width
                    contentHeight: Theme.itemSizeSmall

                    Label {
                        anchors.left: parent.left
                        anchors.leftMargin: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        color: Theme.primaryColor
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.rightMargin: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: Theme.paddingMedium

                        IconButton {
                            icon.source: "image://theme/icon-m-cloud-download"
                            onClicked: apiService.downloadFile(modelData)
                        }
                        IconButton {
                            icon.source: "image://theme/icon-m-delete"
                            onClicked: apiService.deleteFile(modelData)
                        }
                    }
                }
            }
        }
    }

    // Уведомление о завершении скачивания
    Connections {
        target: apiService
        onFileDownloaded: {
            // Можно добавить уведомление (Banner)
            console.log("Файл скачан в: " + filePath)
        }
    }
}
