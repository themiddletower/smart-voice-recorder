import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.auroraos.AudioRecorder 1.0

Page {
    id: page
    objectName: "dictaphonePage"
    allowedOrientations: Orientation.Portrait

    readonly property real lowThreshold: 0.1
    readonly property real highThreshold: 0.85

    property string currentRecordPath: ""
    property real   currentLevel: 0.0
    property bool   isRecording: false
    property bool   isPaused: false
    property bool   isReady: false
    property string volumeWarning: ""
    property string lastError: ""
    property int    recordingDurationMs: 0

    readonly property int measurementsPerHalfSec: 8
    readonly property int timelineBlockWidth: Theme.itemSizeSmall / 2
    readonly property int millisInHalfSec: 500

    property real durationOnPointer:
        (waveformList.contentX + waveformContainer.width / 2)
        * millisInHalfSec / timelineBlockWidth

    onStatusChanged: {
        if (status === PageStatus.Activating)
            playerController.isPlayerPage = false
    }

    function formatDuration(ms) {
        var h = Math.floor(ms / 3600000)
        var m = Math.floor((ms % 3600000) / 60000)
        var s = Math.floor((ms % 60000) / 1000)
        return (h < 10 ? "0" : "") + h + ":"
             + (m < 10 ? "0" : "") + m + ":"
             + (s < 10 ? "0" : "") + s
    }

    function updateVolumeWarning(level) {
        if (level < lowThreshold)
            volumeWarning = qsTr("Говорите громче!")
        else if (level > highThreshold)
            volumeWarning = qsTr("Говорите тише!")
        else
            volumeWarning = ""
    }

    function moveWaveformTo(ms) {
        waveformList.contentX =
            ms / millisInHalfSec * timelineBlockWidth
            - waveformContainer.width / 2
    }

    function resetWaveform() {
        waveformList.positionViewAtBeginning()
        playerController.resetModels()
    }

    AudioRecorderController {
        id: audioRecorder

        onAudioDurationChanged: {
            var level = getAudioLevel()
            currentLevel = level
            recordingDurationMs = duration
            moveWaveformTo(duration)
            if (waveformList.atXEnd)
                playerController.updateModelWithRecorderData(duration, level)
            updateVolumeWarning(level)
        }

        onRecordStarted: {
            isRecording = true
            isPaused = false
            playerController.isPlaybackAvailable = false
        }

        onRecordPaused: {
            isPaused = true
            isRecording = false
            playerController.setSource(currentRecordPath)
        }

        onRecordStopped: {
            isRecording = false
            isPaused = false
            playerController.stop()
            resetWaveform()
            volumeWarning = ""
            currentLevel = 0.0
            recordingDurationMs = 0
        }

        onAudiofilePathChanged: { currentRecordPath = path }
        onRecorderPrepared:     { isReady = true }
        onRecordErrorOccured:   { lastError = error }

        Component.onCompleted: setDefaultRecordSettings()
    }

    Connections {
        target: playerController
        onPositionChanged: {
            if (position > 0) moveWaveformTo(position)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#0F1219"
    }

    Item {
        id: headerArea
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: 110

        // Отмена
        Label {
            anchors {
                left: parent.left; leftMargin: Theme.horizontalPageMargin
                top: parent.top;   topMargin: 60
            }
            text: qsTr("Отмена")
            color: "white"
            font.pixelSize: Theme.fontSizeMedium

            MouseArea {
                anchors.fill: parent; anchors.margins: -Theme.paddingSmall
                onClicked: {
                    if (isRecording || isPaused) audioRecorder.stopRecord()
                    pageStack.pop()
                }
            }
        }

        Label {
            anchors {
                right: parent.right; rightMargin: Theme.horizontalPageMargin
                top: parent.top;     topMargin: 60
            }
            text: qsTr("Готово")
            color: "white"
            font.pixelSize: Theme.fontSizeMedium
            visible: isRecording || isPaused

            MouseArea {
                anchors.fill: parent; anchors.margins: -Theme.paddingSmall
                onClicked: {
                    audioRecorder.stopRecord()
                    pageStack.pop() // Если нужно выходить на предыдущий экран по "Готово"
                }
            }
        }

        Label {
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top; topMargin: 98
            }
            text: qsTr("Новая запись")
            color: "#FFF9F9"
            font { pixelSize: 20; bold: true }
        }
    }

    Label {
        id: timerLabel
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: headerArea.bottom; topMargin: 20
        }
        text: {
            if (isRecording) return formatDuration(recordingDurationMs)
            if (isPaused)    return formatDuration(durationOnPointer)
            return "00:00:00"
        }
        color: "white"
        font { pixelSize: 48; bold: true }
    }

    Rectangle {
        id: waveformContainer
        anchors {
            left: parent.left;  right: parent.right
            top: timerLabel.bottom
            leftMargin: 16; rightMargin: 16; topMargin: Theme.paddingLarge
        }
        height: 100
        radius: 16
        clip: true
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#2A2F3F" }
            GradientStop { position: 1.0; color: "#1A1E2A" }
        }

        ListView {
            id: waveformList
            anchors.fill: parent
            orientation: ListView.Horizontal
            interactive: isPaused
            boundsBehavior: Flickable.StopAtBounds
            model: playerController.audioAmplitudeModel

            property bool isDragPause: false

            header: Item { width: waveformContainer.width / 2 }
            footer: Item { width: waveformContainer.width / 2 }

            delegate: Item {
                width: timelineBlockWidth / measurementsPerHalfSec
                height: waveformList.height

                Rectangle {
                    anchors.centerIn: parent
                    width: 4
                    height: Math.max(4, parent.height * 0.85 * value)
                    radius: 2
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#4CAF50" }
                        GradientStop { position: 1.0; color: "#5CAF91" }
                    }
                }
            }

            onDragStarted: {
                if (playerController.isPlaying) {
                    playerController.stop()
                    isDragPause = true
                }
            }
            onDragEnded: {
                if (isDragPause) {
                    playerController.play(durationOnPointer)
                    isDragPause = false
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 2; height: parent.height
            color: "#FFFFFF"; opacity: 0.4
        }
    }

    Rectangle {
        id: warningBanner
        anchors {
            left: parent.left; right: parent.right
            top: waveformContainer.bottom
            leftMargin: 16; rightMargin: 16; topMargin: Theme.paddingLarge
        }
        height: 48
        radius: 12
        color: "#FFB74D"
        visible: volumeWarning.length > 0

        Label {
            anchors {
                left: parent.left; leftMargin: 12
                verticalCenter: parent.verticalCenter
            }
            text: volumeWarning
            color: "black"
            font { pixelSize: 20; bold: true }
        }
    }

    Item {
        id: levelBar
        anchors {
            left: parent.left; right: parent.right
            top: warningBanner.visible ? warningBanner.bottom
                                       : waveformContainer.bottom
            leftMargin: 16; rightMargin: 16; topMargin: Theme.paddingLarge
        }
        height: 6

        Rectangle {
            anchors.fill: parent; radius: 3
            color: "#2A2F3F"
        }

        Rectangle {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: parent.width * currentLevel
            radius: 3
            color: currentLevel > highThreshold ? "#F44336" : "#4CAF50"

            Behavior on width  { NumberAnimation { duration: 50 } }
            Behavior on color  { ColorAnimation  { duration: 200 } }
        }
    }

    Label {
        anchors {
            left: parent.left; right: parent.right
            top: levelBar.bottom
            margins: Theme.horizontalPageMargin
            topMargin: Theme.paddingMedium
        }
        text: lastError
        visible: lastError.length > 0
        color: "#F44336"
        wrapMode: Text.Wrap
        font.pixelSize: Theme.fontSizeSmall
    }

    Item {
        id: bottomBar
        anchors {
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
            bottomMargin: Theme.paddingLarge
        }
        height: 80

        Item {
            id: recordPauseBtn
            anchors.centerIn: parent
            width: 72; height: 72

            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: isRecording ? "#2A2F3F" : "#E53935"
                Behavior on color { ColorAnimation { duration: 200 } }
            }

            Image {
                anchors.centerIn: parent
                source: "image://theme/icon-m-pause"
                visible: isRecording
                opacity: 0.8
                sourceSize { width: 32; height: 32 }
            }

            MouseArea {
                anchors.fill: parent
                enabled: isReady
                onClicked: {
                    audioRecorder.startRecord()
                }
            }
        }

        Rectangle {
            anchors {
                left: recordPauseBtn.right;
                leftMargin: 32
                verticalCenter: parent.verticalCenter
            }
            width: 56; height: 56
            color: "#1C1B1F"
            radius: 28
            visible: isRecording || isPaused
            Image {
                anchors.centerIn: parent
                source: "image://theme/icon-m-stop"
                sourceSize { width: 28; height: 28 }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    audioRecorder.stopRecord()
                }
            }
        }
    }
}
