import QtQuick 2.0
import Sailfish.Silica 1.0
import Aurora.Controls 1.0

Page {
    id: cloudPage

    property string currentLocalFile: ""
    property string currentMetadata: "{}"
    property string fileToRename: ""

    property bool requestInProgress: false

    property int timeoutRefresh: 30000
    property int timeoutLogin: 60000
    property int timeoutUpload: 200000
    property int timeoutDownload: 300000
    property int timeoutDelete: 60000
    property int timeoutRename: 60000

    function beginRequest(timeoutMs, showLoader) {
        if (showLoader === undefined)
            showLoader = true

        if (showLoader) {
            requestTimeoutTimer.interval = timeoutMs
            requestTimeoutTimer.restart()
            cloudPage.requestInProgress = true
        }
    }

    function finishRequest() {
        requestTimeoutTimer.stop()
        cloudPage.requestInProgress = false
    }

    function showInfo(message) {
        notification.text = message
        notification.color = "#2e7d32"
        notification.show()
    }

    function showError(message) {
        notification.text = message
        notification.color = "#b00020"
        notification.show()
    }

    Timer {
        id: requestTimeoutTimer
        repeat: false
        onTriggered: {
            cloudPage.finishRequest()
            cloudPage.showError("Превышено время ожидания ответа")
        }
    }

    Timer {
        id: autoRefreshTimer
        interval: 15000
        running: true
        repeat: true
        onTriggered: {
            apiService.refreshFileList()
        }
    }

    Rectangle {
        id: notification
        visible: false
        width: parent.width
        height: Theme.itemSizeMedium
        anchors.top: parent.top
        z: 999
        opacity: 0.95

        property alias text: notificationText.text

        function show() {
            visible = true
            hideTimer.restart()
        }

        Label {
            id: notificationText
            anchors.centerIn: parent
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeMedium
        }

        Timer {
            id: hideTimer
            interval: 3500
            repeat: false
            onTriggered: { notification.visible = false }
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: cloudPage.requestInProgress
        color: "#80000000"
        z: 998

        MouseArea {
            anchors.fill: parent
            enabled: cloudPage.requestInProgress
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: cloudPage.requestInProgress
            size: BusyIndicatorSize.Large
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.verticalCenter
            anchors.topMargin: Theme.paddingLarge
            text: "Подождите...\nОбработка запроса"
            horizontalAlignment: Text.AlignHCenter
            color: "white"
        }
    }

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

            SectionHeader {
                text: "Авторизация"
            }

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
                    enabled: !cloudPage.requestInProgress
                    onClicked: {
                        if (userField.text === "" || passField.text === "") {
                            cloudPage.showError("Введите логин и пароль")
                            return
                        }
                        cloudPage.beginRequest(timeoutLogin, true)
                        apiService.login(userField.text, passField.text)
                    }
                }

                Button {
                    text: "Регистрация"
                    enabled: !cloudPage.requestInProgress
                    onClicked: {
                        if (userField.text === "" || passField.text === "") {
                            cloudPage.showError("Введите логин и пароль")
                            return
                        }
                        cloudPage.beginRequest(timeoutLogin, true)
                        apiService.registerUser(userField.text, passField.text)
                    }
                }
            }

            Separator {
                width: parent.width
                color: Theme.primaryColor
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Выгрузить текущий файл"
                enabled: cloudPage.currentLocalFile !== "" && !cloudPage.requestInProgress
                onClicked: {
                    cloudPage.beginRequest(timeoutUpload, true)
                    apiService.sendDataToServer(cloudPage.currentLocalFile, cloudPage.currentMetadata)
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Обновить список облака"
                enabled: !cloudPage.requestInProgress
                onClicked: {
                    cloudPage.beginRequest(timeoutRefresh, true)
                    apiService.refreshFileList()
                }
            }

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
                            enabled: !cloudPage.requestInProgress
                            onClicked: {
                                cloudPage.beginRequest(timeoutDownload, true)
                                apiService.downloadFile(modelData)
                            }
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-delete"
                            enabled: !cloudPage.requestInProgress
                            onClicked: {
                                cloudPage.beginRequest(timeoutDelete, true)
                                apiService.deleteFile(modelData)
                            }
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-edit"
                            enabled: !cloudPage.requestInProgress
                            onClicked: {
                                cloudPage.fileToRename = modelData
                                renameDialog.open()
                            }
                        }
                    }
                }
            }

            Dialog {
                id: renameDialog
                Column {
                    width: parent.width
                    spacing: Theme.paddingMedium

                    Label {
                        text: "Новое имя файла"
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                    }
                    TextField {
                        id: newNameField
                        width: parent.width
                    }
                    Button {
                        text: "Переименовать"
                        anchors.horizontalCenter: parent.horizontalCenter
                        onClicked: {
                            if (newNameField.text === "") {
                                cloudPage.showError("Введите имя файла")
                                return
                            }
                            cloudPage.beginRequest(timeoutRename, true)
                            newNameField.focus = false
                            var ext = cloudPage.fileToRename.split(".").pop()
                            var newName = newNameField.text + "." + ext
                            apiService.renameFile(cloudPage.fileToRename, newName)
                            renameDialog.accept()
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: apiService

        onFileListChanged: {
            cloudPage.finishRequest()
        }

        onLoginSuccess: {
            cloudPage.finishRequest()
            cloudPage.showInfo("Авторизация успешна")
        }

        onLoginError: {
            cloudPage.finishRequest()
            cloudPage.showError(errorText)
        }

        onRegisterSuccess: {
            cloudPage.finishRequest()
            cloudPage.showInfo("Регистрация успешна. Войдите")
        }
        onRegisterError: {
            cloudPage.finishRequest()
            cloudPage.showError(errorText)
        }

        onUploadSuccess: {
            cloudPage.finishRequest()
            cloudPage.showInfo("Файл загружен")
        }

        onFileDownloaded: {
            cloudPage.finishRequest()
            cloudPage.showInfo("Скачан:\n" + filePath)
        }

        onApiError: {
            cloudPage.finishRequest()
            cloudPage.showError(errorText)
        }
    }
}
