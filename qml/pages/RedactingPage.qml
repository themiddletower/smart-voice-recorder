import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0 // Добавляем импорт для работы с файлами
Page {
    objectName: "redactingPage"
    allowedOrientations: Orientation.All

    property string selectedFilePath: ""
    property string fileName

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: layout.height + Theme.paddingLarge

        Column {
            id: layout
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Redacting Page")
            }

            Label {
                text: qsTr("Вы сейчас находитесь на странице редактирования звука")
                anchors.horizontalCenter: parent.horizontalCenter
                color: palette.highlightColor
            }

            Button {
                text: qsTr("Выбрать звуковой файл")
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !record  // Отключаем, если уже записываем
                onClicked: {
                    //record = 1;
                    //pageStack.push(Qt.resolvedUrl("MarkAttendancePage.qml"))
                    pageStack.push(filePickerPage)

                }
            }
            Label {
                visible: selectedFilePath !== ""
                text: "Выбранный файл: " + selectedFilePath
                width: parent.width
                wrapMode: Text.Wrap
                color: Theme.highlightColor
            }

            Button {
                text: "Открыть встроенный mp3"
                    onClicked: {
                    selectedFilePath = ":/backend/example.mp3"
                    }
                }

            Label {
                visible: selectedFilePath !== ""
                text: "Выбранный файл: " + selectedFilePath
                width: parent.width
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                }

            Button {
                text: qsTr("Удаление пауз")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    //sessionManager Или ваш cpp файл.deletePause(файлик);
                }
                enabled: selectedFilePath !== ""
            }

            Button {
                text: qsTr("Сегментирование речевых моментов")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    //sessionManager Или ваш cpp файл.segmentSound(файлик);
                }
                enabled: selectedFilePath !== ""
            }
            Button {
                text: qsTr("Вернуться на главную страницу")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(Qt.resolvedUrl("MainPage.qml"))
                }
        }
    }
    Component {
                    id: filePickerPage
                    FilePickerPage {
                        title: qsTr("Выберите ваш звуковой файл")
                        nameFilters: [ '*.mp3' ]
                        onSelectedContentPropertiesChanged: {
                            if (selectedContentProperties !== null) {
                                selectedFilePath = selectedContentProperties.filePath
                                fileName = selectedContentProperties.fileName
                            }
                        }
                    }
                }
}
