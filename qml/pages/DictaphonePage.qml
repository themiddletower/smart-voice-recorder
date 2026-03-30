import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.auroraos.AudioRecorder 1.0
import "../components"

Page {
    objectName: "dictaphonePage"
    allowedOrientations: Orientation.Portrait

    readonly property real lowThreshold: 0.1
    readonly property real highThreshold: 0.85

    property string currentRecordPath: ""
    property real currentLevel: 0.0
    property bool isRecording: false
    property bool isPaused: false
    property bool isReady: false
    property string volumeWarning: ""

    onStatusChanged: {
        if (status === PageStatus.Activating)
            playerController.isPlayerPage = false
    }

    AudioRecorderController {
        id: audioRecorder

        onAudioDurationChanged: {
            var level = getAudioLevel()
            currentLevel = level
            recordTrack.newRecorderDataRecieved(duration, level)
            updateVolumeWarning(level)
        }

        onRecordStarted: {
            console.log("Record started!")
            isRecording = true
            isPaused = false
            playerController.isPlaybackAvailable = false
            recordTrack.isFlickable = false
        }

        onRecordPaused: {
            console.log("Record paused!")
            isPaused = true
            isRecording = false
            playerController.setSource(currentRecordPath)
            recordTrack.isFlickable = true
        }

        onRecordStopped: {
            console.log("Record stopped!")
            isRecording = false
            isPaused = false
            playerController.stop()
            recordTrack.reset()
            volumeWarning = ""
            currentLevel = 0.0
        }

        onAudiofilePathChanged: {
            console.log("File path: " + path)
            currentRecordPath = path
        }

        onRecorderPrepared: {
            console.log("Recorder prepared!")
            isReady = true
        }

        onRecordErrorOccured: {
            console.log("Record error: " + error)
            errorMsg.visible = true
            errorMsg.text = error
            buttonRow.visible = false
        }

        Component.onCompleted: {
            console.log("Setting default record settings...")
            setDefaultRecordSettings()
            console.log("Default settings applied")
        }
    }

    function updateVolumeWarning(level) {
        if (level < lowThreshold)
            volumeWarning = qsTr("Слишком тихо, говорите громче")
        else if (level > highThreshold)
            volumeWarning = qsTr("Слишком громко, говорите тише")
        else
            volumeWarning = ""
    }

    PageHeader {
        id: pageHeader
        title: qsTr("Диктофон")
    }

    Item {
        id: pageRoot
        anchors {
            left: parent.left
            right: parent.right
            top: pageHeader.bottom
            bottom: parent.bottom
        }

        RecordTrack {
            id: recordTrack
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: volumeColumn.top
                topMargin: Theme.paddingLarge
                bottomMargin: Theme.paddingMedium
            }
        }

        Column {
            id: volumeColumn
            anchors {
                left: parent.left
                right: parent.right
                bottom: warningLabel.top
                margins: Theme.horizontalPageMargin
            }
            spacing: Theme.paddingSmall

            Label {
                text: qsTr("Уровень громкости")
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                width: parent.width
                height: Theme.itemSizeExtraSmall / 2

                Rectangle {
                    anchors.fill: parent
                    radius: height / 2
                    color: Theme.rgba(Theme.highlightColor, 0.2)
                }

                Rectangle {
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                    }
                    width: parent.width * currentLevel
                    radius: height / 2
                    color: {
                        if (currentLevel < lowThreshold)
                            return "#4CAF50"
                        else if (currentLevel > highThreshold)
                            return "#F44336"
                        else
                            return "#2196F3"
                    }

                    Behavior on width {
                        NumberAnimation { duration: 50; easing.type: Easing.OutQuad }
                    }
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                Rectangle {
                    x: parent.width * lowThreshold
                    anchors { top: parent.top; bottom: parent.bottom }
                    width: 1
                    color: Theme.rgba("white", 0.3)
                }
                Rectangle {
                    x: parent.width * highThreshold
                    anchors { top: parent.top; bottom: parent.bottom }
                    width: 1
                    color: Theme.rgba("white", 0.3)
                }
            }

            Label {
                text: Math.round(currentLevel * 100) + "%"
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        Label {
            id: warningLabel
            anchors {
                left: parent.left
                right: parent.right
                bottom: timePassed.top
                margins: Theme.horizontalPageMargin
            }
            horizontalAlignment: Text.AlignHCenter
            text: volumeWarning
            visible: volumeWarning.length > 0
            font.pixelSize: Theme.fontSizeMedium
            font.bold: true
            wrapMode: Text.WordWrap
            color: currentLevel < lowThreshold ? "#FFC107" : "#F44336"

            SequentialAnimation on opacity {
                running: volumeWarning.length > 0
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 500 }
                NumberAnimation { to: 1.0; duration: 500 }
            }
        }

        Label {
            id: errorMsg
            wrapMode: Text.Wrap
            anchors {
                bottom: timePassed.top
                horizontalCenter: pageRoot.horizontalCenter
                margins: Theme.horizontalPageMargin
            }
            visible: false
            color: "#F44336"
        }

        Label {
            id: timePassed
            anchors {
                bottom: buttonRow.top
                horizontalCenter: pageRoot.horizontalCenter
                margins: Theme.horizontalPageMargin
            }
            text: recordTrack.timeString
            font.pixelSize: Theme.fontSizeMedium * 2
            color: isRecording ? Theme.highlightColor : Theme.primaryColor
        }

        Row {
            id: buttonRow
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                margins: Theme.horizontalPageMargin
            }
            height: Theme.iconSizeExtraLarge + recordLabel.height + Theme.paddingSmall
            spacing: Theme.itemSizeSmall

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.paddingSmall

                IconButton {
                    id: recordButton
                    anchors.horizontalCenter: parent.horizontalCenter
                    icon {
                        source: isRecording
                            ? "image://theme/icon-l-opaque-pause"
                            : "image://theme/icon-m-call-recording-on-dark"
                        width: Theme.iconSizeExtraLarge
                        height: Theme.iconSizeExtraLarge
                    }
                    width: Theme.iconSizeExtraLarge
                    height: Theme.iconSizeExtraLarge
                    enabled: isReady
                    onClicked: {
                        console.log("Record button clicked!")
                        audioRecorder.startRecord()
                    }
                }

                Label {
                    id: recordLabel
                    text: {
                        if (isRecording) return qsTr("Пауза")
                        else if (isPaused) return qsTr("Продолжить")
                        else return qsTr("Запись")
                    }
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.paddingSmall
                visible: isRecording || isPaused

                IconButton {
                    id: stopButton
                    anchors.horizontalCenter: parent.horizontalCenter
                    icon {
                        source: "image://theme/icon-m-stop"
                        width: Theme.iconSizeMedium
                        height: Theme.iconSizeMedium
                    }
                    width: Theme.iconSizeLarge
                    height: Theme.iconSizeLarge
                    onClicked: {
                        console.log("Stop button clicked!")
                        audioRecorder.stopRecord()
                    }
                }

                Label {
                    text: qsTr("Стоп")
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
