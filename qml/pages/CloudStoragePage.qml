import QtQuick 2.0
import Sailfish.Silica 1.0
import Aurora.Controls 1.0

Page {
    id: cloudPage

    property string currentLocalFile: ""

    // ПОЛНЫЙ JSON:
    // {
    //   annotations: [],
    //   voiceLabels: []
    // }
    property string currentMetadata: "{}"

    property string fileToRename: ""

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Облачное хранилище"
            }

            // =====================================================
            // AUTH
            // =====================================================

            SectionHeader {
                text: "Авторизация"
            }

            TextField {
                id: userField

                width: parent.width -
                       Theme.horizontalPageMargin * 2

                anchors.horizontalCenter:
                    parent.horizontalCenter

                placeholderText: "Логин"
                label: "Пользователь"

                text: apiService.getSavedUsername()
            }

            PasswordField {
                id: passField

                width: parent.width -
                       Theme.horizontalPageMargin * 2

                anchors.horizontalCenter:
                    parent.horizontalCenter

                placeholderText: "Пароль"
                label: "Пароль"

                text: apiService.getSavedPassword()
            }

            Row {
                anchors.horizontalCenter:
                    parent.horizontalCenter

                spacing: Theme.paddingMedium

                Button {
                    text: "Вход"

                    onClicked: {
                        apiService.login(
                            userField.text,
                            passField.text
                        )
                    }
                }

                Button {
                    text: "Регистрация"

                    onClicked: {
                        apiService.registerUser(
                            userField.text,
                            passField.text
                        )
                    }
                }
            }

            // =====================================================
            // FILE ACTIONS
            // =====================================================

            Separator {
                width: parent.width
                color: Theme.primaryColor
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter:
                    parent.horizontalCenter

                text: "Выгрузить текущий файл"

                enabled:
                    cloudPage.currentLocalFile !== ""

                onClicked: {

                    apiService.sendDataToServer(
                        cloudPage.currentLocalFile,
                        cloudPage.currentMetadata
                    )
                }
            }

            Button {
                anchors.horizontalCenter:
                    parent.horizontalCenter

                text: "Обновить список облака"

                onClicked: {
                    apiService.refreshFileList()
                }
            }

            // =====================================================
            // CLOUD FILES
            // =====================================================

            SectionHeader {
                text: "Файлы в облаке"
            }

            Repeater {

                model: apiService.fileListModel

                delegate: ListItem {

                    width: cloudPage.width
                    contentHeight: Theme.itemSizeSmall

                    Label {

                        anchors.left: parent.left

                        anchors.leftMargin:
                            Theme.horizontalPageMargin

                        anchors.verticalCenter:
                            parent.verticalCenter

                        text: modelData

                        color: Theme.primaryColor
                    }

                    Row {

                        anchors.right: parent.right

                        anchors.rightMargin:
                            Theme.horizontalPageMargin

                        anchors.verticalCenter:
                            parent.verticalCenter

                        spacing: Theme.paddingMedium

                        // DOWNLOAD
                        IconButton {

                            icon.source:
                                "image://theme/icon-m-cloud-download"

                            onClicked: {
                                apiService.downloadFile(
                                    modelData
                                )
                            }
                        }

                        // DELETE
                        IconButton {

                            icon.source:
                                "image://theme/icon-m-delete"

                            onClicked: {
                                apiService.deleteFile(
                                    modelData
                                )
                            }
                        }

                        // RENAME
                        IconButton {

                            icon.source:
                                "image://theme/icon-m-edit"

                            onClicked: {

                                cloudPage.fileToRename =
                                        modelData

                                renameDialog.open()
                            }
                        }
                    }
                }
            }

            // =====================================================
            // RENAME DIALOG
            // =====================================================

            Dialog {
                id: renameDialog

                Column {

                    width: parent.width

                    spacing: Theme.paddingMedium

                    Label {

                        text: "Новое имя файла"

                        width: parent.width

                        horizontalAlignment:
                            Text.AlignHCenter
                    }

                    TextField {
                        id: newNameField
                        width: parent.width
                    }

                    Button {

                        text: "Переименовать"

                        anchors.horizontalCenter:
                            parent.horizontalCenter

                        onClicked: {

                            newNameField.focus = false

                            var ext =
                                    cloudPage.fileToRename
                                    .split(".")
                                    .pop()

                            var newName =
                                    newNameField.text +
                                    "." +
                                    ext

                            apiService.renameFile(
                                cloudPage.fileToRename,
                                newName
                            )

                            renameDialog.accept()
                        }
                    }
                }
            }
        }
    }

    Connections {

        target: apiService

        function onFileDownloaded(filePath)
        {
            console.log(
                "Файл скачан: " + filePath
            )
        }
    }
}
