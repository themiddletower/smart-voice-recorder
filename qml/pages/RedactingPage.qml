import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.auroraos.AudioRecorder 1.0
import Aurora.Controls 1.0
import Sailfish.Pickers 1.0
import "../components"

Page {
    id: page
    objectName: "redactingPage"

    property string filePath: ""
    property alias fileName: header.headerText

    property var fileAnnotations: [
        {"t1": 500, "t2": 2500, "type": 1},
        {"t1": 2500, "t2": 4000, "type": 2},
        {"t1": 4000, "t2": 7200, "type": 1},
        {"t1": 7200, "t2": 10700, "type": 2},
        {"t1": 10700, "t2": 13000, "type": 1},
        {"t1": 12000, "t2": 15000, "type": 3}
    ]

    // должно совпадать с C++ и формулами
    readonly property int measurementsPerSec: 10
    readonly property int barWidthPx: 6

    Component.onCompleted: playerController.isPlayerPage = true

    onStatusChanged: {
        if (status === PageStatus.Deactivating)
            playerController.stop()
    }

    Connections {
        target: playerController
        onDecodingCompleted: {
            playerController.audioAmplitudeModel.applyAnnotations(fileAnnotations, measurementsPerSec)
        }
    }

    AppBar {
        id: header
        headerText: filePath ? filePath.split('/').pop() : "Выберите аудиофайл"

        IconButton {
            icon.source: "image://theme/icon-m-folder"
            onClicked: openFilePicker()
        }
    }

    Item {
        id: pageRoot
        anchors {
            left: parent.left; right: parent.right
            top: header.bottom; bottom: parent.bottom
        }

        function seekToMs(ms) {
            if (ms < 0) ms = 0
            var wasPlaying = playerController.isPlaying
            playerController.play(ms)
            if (!wasPlaying)
                playerController.stop()
        }

        Item {
            id: waveformContainer
            anchors {
                left: parent.left; right: parent.right
                top: parent.top; bottom: timePassed.top
                margins: Theme.paddingLarge
            }
            visible: filePath !== "" && !playerController.isDecoding
            clip: true

            // фоновые линии заметок
            Item {
                width: waveformList.contentWidth
                height: parent.height
                x: -waveformList.contentX

                Repeater {
                    model: page.fileAnnotations
                    Item {
                        x: (modelData.t1 / 1000.0) * measurementsPerSec * barWidthPx
                        width: ((modelData.t2 - modelData.t1) / 1000.0) * measurementsPerSec * barWidthPx
                        height: parent.height

                        Rectangle {
                            anchors.left: parent.left
                            width: 2
                            height: parent.height
                            color: getColorForType(modelData.type)
                            opacity: 0.8
                        }
                        Rectangle {
                            anchors.right: parent.right
                            width: 2
                            height: parent.height
                            color: getColorForType(modelData.type)
                            opacity: 0.8
                        }
                    }
                }
            }

            ListView {
                id: waveformList
                anchors.fill: parent
                orientation: ListView.Horizontal
                model: playerController.audioAmplitudeModel
                boundsBehavior: Flickable.StopAtBounds

                Connections {
                    target: playerController
                    onPositionChanged: {
                        if (playerController.isPlaying) {
                            var currentItemIndex = Math.floor((playerController.position / 1000) * measurementsPerSec)
                            waveformList.positionViewAtIndex(currentItemIndex, ListView.Contain)
                        }
                    }
                }

                delegate: Item {
                    width: barWidthPx
                    height: waveformList.height

                    Rectangle {
                        id: bar
                        anchors.centerIn: parent
                        width: 4
                        height: Math.max(4, model.value * parent.height)
                        radius: 2
                        color: getColorForType(model.annotationType)
                    }

                    // ТАП ПО БАРУ -> SEEK
                    // Скролл не ломаем, потому что MouseArea только на баре,
                    // и Flickable всё равно умеет перехватывать drag.
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // index = номер бара
                            var ms = (index / measurementsPerSec) * 1000.0
                            pageRoot.seekToMs(ms)
                        }
                    }
                }
            }

            // плейхед
            Rectangle {
                width: 2
                height: parent.height
                color: "red"
                x: ((playerController.position / 1000.0) * measurementsPerSec * barWidthPx) - waveformList.contentX
                visible: x > 0 && x < parent.width
            }
        }

        BusyIndicator {
            anchors.centerIn: waveformContainer
            size: BusyIndicatorSize.Large
            running: playerController.isDecoding
            visible: playerController.isDecoding
        }

        Label {
            id: timePassed
            anchors {
                bottom: playButton.top
                horizontalCenter: parent.horizontalCenter
                margins: Theme.horizontalPageMargin
            }
            text: playerController.pointerPositionToString(playerController.position)
            font.pixelSize: Theme.fontSizeMedium * 2
            visible: filePath !== "" && !playerController.isDecoding
        }

        IconButton {
            id: playButton
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                margins: Theme.horizontalPageMargin
            }
            icon {
                source: playerController.isPlaying ? "image://theme/icon-m-pause"
                                                   : "image://theme/icon-m-simple-play"
                width: Theme.iconSizeLarge
                height: Theme.iconSizeLarge
            }
            height: icon.height
            width: icon.width
            enabled: playerController.isPlaybackAvailable && filePath !== "" && !playerController.isDecoding
            visible: filePath !== ""

            onClicked: {
                if (playerController.isPlaying) {
                    playerController.stop()
                } else {
                    var startPosMs = (waveformList.contentX / barWidthPx / measurementsPerSec) * 1000
                    startPosMs = Math.max(0, startPosMs)
                    playerController.play(startPosMs)
                }
            }
        }

        Label {
            anchors.centerIn: parent
            text: "Выберите аудиофайл\nнажмите на иконку папки вверху"
            horizontalAlignment: Text.AlignHCenter
            color: Theme.secondaryColor
            visible: filePath === ""
            font.pixelSize: Theme.fontSizeMedium
        }
    }

    function openFilePicker() {
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", {
            nameFilters: ["*.wav", "*.mp3", "*.ogg", "*.flac"],
            title: "Выберите аудиофайл"
        })

        dialog.selectedContentPropertiesChanged.connect(function() {
            if (dialog.selectedContentProperties && dialog.selectedContentProperties.filePath) {
                var selectedPath = dialog.selectedContentProperties.filePath
                filePath = selectedPath
                header.headerText = dialog.selectedContentProperties.fileName || selectedPath.split('/').pop()
                playerController.setSource(selectedPath)
                pageStack.pop()
            }
        })
    }

    function getColorForType(typeCode) {
        switch (typeCode) {
            case 1: return "lime"
            case 2: return "gray"
            case 3: return "red"
            default: return Theme.primaryColor
        }
    }
}
