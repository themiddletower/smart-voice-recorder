import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.auroraos.AudioRecorder 1.0

Item {
    id: root

    property real durationOnPointer: (audioLevel.contentX + (root.width / 2)) *
                                     style.millisInHalfSec / style.timelineBlockWidth
    property bool isFlickable: false
    property string timeString: playerController.pointerPositionToString(durationOnPointer)

    signal newRecorderDataRecieved(real duration, real amplitude)

    QtObject {
        id: style
        readonly property color componentColor: "#A5D2FF"
        readonly property real componentOpacity: 0.4
        readonly property int graphicColumnWidth: 2
        readonly property int amplitudeMeasurementPerHalfSec: 8
        readonly property int timelineBlockWidth: Theme.itemSizeSmall / 2
        readonly property int pointerEdgeHeight: 20
        readonly property int millisInHalfSec: 500
    }

    function reset() {
        audioLevel.positionViewAtBeginning()
        isFlickable = false
        playerController.resetModels()
    }

    function moveTo(duration) {
        audioLevel.contentX = duration / style.millisInHalfSec *
                style.timelineBlockWidth - (root.width / 2)
    }

    onNewRecorderDataRecieved: {
        moveTo(duration)
        if (audioLevel.atXEnd)
            playerController.updateModelWithRecorderData(duration, amplitude)
    }

    Connections {
        target: playerController
        onPositionChanged: {
            if (position > 0) moveTo(position)
        }
    }

    Rectangle {
        id: audioLevelPlaceholder
        anchors {
            fill: parent
            bottomMargin: Theme.itemSizeSmall
        }
        color: style.componentColor
        opacity: style.componentOpacity
    }

    Rectangle {
        id: pointer
        anchors.centerIn: audioLevelPlaceholder
        height: audioLevelPlaceholder.height
        width: style.graphicColumnWidth
        color: style.componentColor

        Rectangle {
            anchors {
                top: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
            height: style.pointerEdgeHeight
            width: height
            radius: height / 2
            color: parent.color
        }

        Rectangle {
            anchors {
                bottom: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            height: style.pointerEdgeHeight
            width: height
            radius: height / 2
            color: parent.color
        }
    }

    SilicaListView {
        id: audioLevel
        anchors.fill: parent
        orientation: ListView.Horizontal
        interactive: isFlickable
        model: playerController.audioAmplitudeModel

        footer: Item { width: root.width / 2 }
        header: Item { width: root.width / 2 }

        delegate: Item {
            y: (audioLevelPlaceholder.height - height) / 2
            height: root.height / 5
            width: style.timelineBlockWidth / style.amplitudeMeasurementPerHalfSec
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: style.graphicColumnWidth
                height: parent.height * value
                color: "#FFFFFF"
            }
        }
    }

    SilicaListView {
        id: timeLine
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: Theme.itemSizeSmall
        orientation: ListView.Horizontal
        interactive: false
        contentX: audioLevel.contentX
        model: playerController.timelineModel
        header: Item { width: root.width / 2 }

        delegate: Item {
            width: style.timelineBlockWidth
            height: parent.height

            Rectangle {
                id: separator
                anchors.left: parent.left
                height: isDisplayTime ? parent.height / 2 : parent.height / 3
                width: style.graphicColumnWidth
                color: style.componentColor
                opacity: style.componentOpacity
            }

            Label {
                anchors {
                    horizontalCenter: separator.horizontalCenter
                    top: separator.bottom
                }
                text: time
                visible: isDisplayTime
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }
    }
}
