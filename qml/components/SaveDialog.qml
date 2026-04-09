import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: saveDialog
    property string initialName: ""
    property string extension: ""
    property string newName: nameField.text

    Column {
        width: parent.width
        spacing: Theme.paddingLarge

        DialogHeader {
            title: "Сохранить файл"
            acceptText: "Сохранить"
        }

        TextField {
            id: nameField
            width: parent.width - 2 * Theme.horizontalPageMargin
            anchors.horizontalCenter: parent.horizontalCenter
            label: "Имя файла"
            text: initialName
            placeholderText: "Введите имя файла"
            focus: true
            EnterKey.enabled: text.length > 0
            EnterKey.onClicked: saveDialog.accept()
        }

        Label {
            text: "Расширение: " + extension
            color: Theme.secondaryColor
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
