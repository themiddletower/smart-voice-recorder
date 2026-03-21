import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    objectName: "mainPage"
    allowedOrientations: Orientation.All

    /*
    PageHeader {
        objectName: "pageHeader"
        title: qsTr("Template")
        extraContent.children: [
            IconButton {
                objectName: "aboutButton"
                icon.source: "image://theme/icon-m-about"
                anchors.verticalCenter: parent.verticalCenter

                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        ]
    }
    */
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: layout.height + Theme.paddingLarge

        Column {
            id: layout
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Main Page")
            }

            Label {
                text: qsTr("Вы сейчас находитесь на главной странице")
                anchors.horizontalCenter: parent.horizontalCenter
                color: palette.highlightColor
            }

            Button {
                text: qsTr("Запись на диктофон")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {//pageStack.push(Qt.resolvedUrl("RecordingPage.qml"))
                    var component = Qt.resolvedUrl("RecordingPage.qml")
                    console.log("Loading page from:", component)
                    pageStack.push(component)
                }
            }

            Button {
                text: qsTr("Редактирование записи")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(Qt.resolvedUrl("RedactingPage.qml"))
            }

        }
    }
}
