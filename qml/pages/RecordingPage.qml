import QtQuick 2.0
import Sailfish.Silica 1.0
Page {
    objectName: "recordingPage"
    allowedOrientations: Orientation.All

    property bool record: false

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: layout.height + Theme.paddingLarge

        Column {
            id: layout
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Recording Page")
            }

            Label {
                text: qsTr("Вы сейчас находитесь на странице записи звука")
                anchors.horizontalCenter: parent.horizontalCenter
                color: palette.highlightColor
            }

            Button {
                text: qsTr("Запись")
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !record
                onClicked: {
                    record = 1;
                }
            }

            Button {
                text: qsTr("Остановка записи")
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: record
                onClicked: {
                    record = 0;
                }

            }

            Button {
                text: qsTr("Вернуться на главную страницу")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(Qt.resolvedUrl("MainPage.qml"))
                }
        }
    }
}
