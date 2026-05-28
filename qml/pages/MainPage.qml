import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    objectName: "mainPage"
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: layout.height + Theme.paddingLarge

        Column {
            id: layout
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                //title: qsTr("Аудио Студия")
            }

            Item {
                width: parent.width
                height: iconLarge.height + titleLabel.height + Theme.paddingMedium

                Icon {
                    id: iconLarge
                    source: "image://theme/icon-l-music"
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: Theme.highlightColor
                }

                Label {
                    id: titleLabel
                    anchors.top: iconLarge.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Главная")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeExtraLarge
                }
            }

            Item {
                width: parent.width
                height: Theme.paddingLarge
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - Theme.horizontalPageMargin * 2
                text: qsTr("Начать запись")
                icon.source: "image://theme/icon-m-micro-phone"

                color: Theme.primaryColor
                backgroundColor: Theme.rgba(Theme.highlightBackgroundColor, 0.15)

                onClicked: pageStack.push(Qt.resolvedUrl("DictaphonePage.qml"))
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - Theme.horizontalPageMargin * 2
                text: qsTr("Редактирование")
                icon.source: "image://theme/icon-m-edit"

                color: Theme.secondaryHighlightColor

                onClicked: pageStack.push(Qt.resolvedUrl("RedactingPage.qml"))
            }

            Separator {
                width: parent.width - Theme.horizontalPageMargin * 2
                anchors.horizontalCenter: parent.horizontalCenter
                color: Theme.highlightColor
                horizontalAlignment: Qt.AlignHCenter
            }

            Label {
                text: qsTr("Приложение для работы со звуком")
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }
        }
    }
}
