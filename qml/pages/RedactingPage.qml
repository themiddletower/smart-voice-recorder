import QtQuick 2.0
import Sailfish.Silica 1.0
import Aurora.Controls 1.0
import Sailfish.Pickers 1.0
import ru.auroraos.AudioRecorder 1.0
import "../components"

Page {
    id: page
    objectName: "mainPage"

    readonly property int silence_type: 2
    readonly property int measurementsPerSec: 10
    readonly property int barWidthPx: 6
    readonly property int pxPerSecond: measurementsPerSec * barWidthPx

    property string filePath: ""
    property alias fileName: header.headerText

    // Добавлена переменная для моментального отслеживания курсора (без задержек плеера)
    property real currentPosMs: 0

    property var fileAnnotations: [
        {"t1": 500, "t2": 2300, "type": 1},
        {"t1": 2300, "t2": 4000, "type": 2},
        {"t1": 4000, "t2": 7000, "type": 1},
        {"t1": 7000, "t2": 10800, "type": 2},
        {"t1": 10800, "t2": 12700, "type": 1},
        {"t1": 12700, "t2": 14100, "type": 2},
        {"t1": 14100, "t2": 17300, "type": 1},
        {"t1": 17300, "t2": 18800, "type": 2},
    ]

    property var voiceLabels: []
    property var annotationsWithIds: []

    property string lastError: ""

    SilenceService { id: silenceService }

    function rebuildVoiceIdsAndTitles() {
        var voiceCounter = 0
        var result = []
        for (var i = 0; i < fileAnnotations.length; i++) {
            var a = fileAnnotations[i]
            var obj = { "t1": a.t1, "t2": a.t2, "type": a.type }
            if (a.type === 1) {
                voiceCounter++
                obj.voiceId = voiceCounter
            }
            result.push(obj)
        }
        annotationsWithIds = result

        var newLabels = []
        for (var id = 1; id <= voiceCounter; id++) {
            var found = null
            for (var j = 0; j < voiceLabels.length; j++) {
                if (voiceLabels[j].voiceId === id) { found = voiceLabels[j]; break }
            }
            if (found) newLabels.push(found)
            else newLabels.push({ "voiceId": id, "title": "человеческая речь " + id })
        }
        voiceLabels = newLabels
    }

    function titleForVoiceId(voiceId) {
        for (var i = 0; i < voiceLabels.length; i++) {
            if (voiceLabels[i].voiceId === voiceId) return voiceLabels[i].title
        }
        return "человеческая речь " + voiceId
    }

    function setTitleForVoiceId(voiceId, newTitle) {
        var t = (newTitle || "").trim()
        if (t.length === 0) t = "человеческая речь " + voiceId
        for (var i = 0; i < voiceLabels.length; i++) {
            if (voiceLabels[i].voiceId === voiceId) {
                voiceLabels[i].title = t
                voiceLabels = voiceLabels
                return
            }
        }
    }

    function removeSilenceNow() {
        if (!filePath || filePath === "") return
        lastError = ""
        playerController.pause()

        var r = silenceService.removeSilence(filePath, fileAnnotations, silence_type)
        processCutResult(r)
    }

    function deleteSegmentUnderCursor() {
        if (!filePath || filePath === "") return

        // Используем нашу синхронную позицию!
        var pos = currentPosMs
        var targetIndex = -1

        for (var i = 0; i < fileAnnotations.length; i++) {
            // Строго меньше (< t2), чтобы на границе двух отрезков выбирался правильный
            if (pos >= fileAnnotations[i].t1 && pos < fileAnnotations[i].t2) {
                targetIndex = i
                break
            }
        }

        // Если кликнули в самый конец последнего сегмента
        if (targetIndex === -1 && fileAnnotations.length > 0) {
            var last = fileAnnotations[fileAnnotations.length - 1]
            if (pos === last.t2) targetIndex = fileAnnotations.length - 1
        }

        if (targetIndex === -1) {
            lastError = "Нет сегмента под курсором (поз: " + Math.round(pos) + "мс)"
            return
        }

        lastError = ""
        playerController.pause()

        var fakeAnnotations = JSON.parse(JSON.stringify(fileAnnotations))
        fakeAnnotations[targetIndex].type = 999

        var r = silenceService.removeSilence(filePath, fakeAnnotations, 999)
        processCutResult(r)
    }

    function processCutResult(r) {
        if (!r || r.error) {
            lastError = r ? r.error : "Ошибка: пустой ответ"
            return
        }

        var raw = r.annotations
        var clean = []

        for (var i = 0; i < raw.length; i++) {
            var cur = raw[i]

            // Игнорируем куски с нулевой или отрицательной длиной
            if (cur.t1 >= cur.t2) continue

            // ПРОСТО ДОБАВЛЯЕМ (без склеивания соседних фрагментов!)
            clean.push({ "t1": cur.t1, "t2": cur.t2, "type": cur.type })
        }

        filePath = r.outputPath
        header.headerText = filePath ? filePath.split('/').pop() : ""

        // Применяем аннотации
        fileAnnotations = clean
        rebuildVoiceIdsAndTitles()

        playerController.setSource(filePath)
        playerController.audioAmplitudeModel.applyAnnotations(fileAnnotations, measurementsPerSec)
    }

    function seekToNextSegment() {
        var currentPos = currentPosMs
        var nextPos = -1
        for (var i = 0; i < fileAnnotations.length; i++) {
            if (fileAnnotations[i].t1 > currentPos + 50) {
                if (nextPos === -1 || fileAnnotations[i].t1 < nextPos) {
                    nextPos = fileAnnotations[i].t1
                }
            }
        }
        if (nextPos !== -1) pageRoot.seekToMs(nextPos)
    }

    function seekToPrevSegment() {
        var currentPos = currentPosMs
        var prevPos = -1
        for (var i = 0; i < fileAnnotations.length; i++) {
            if (fileAnnotations[i].t1 < currentPos - 50) {
                if (prevPos === -1 || fileAnnotations[i].t1 > prevPos) {
                    prevPos = fileAnnotations[i].t1
                }
            }
        }
        if (prevPos !== -1) pageRoot.seekToMs(prevPos)
        else pageRoot.seekToMs(0)
    }

    Component.onCompleted: {
        playerController.isPlayerPage = true
        rebuildVoiceIdsAndTitles()
    }

    onFileAnnotationsChanged: rebuildVoiceIdsAndTitles()

    Connections {
        target: playerController
        onDecodingCompleted: {
            playerController.audioAmplitudeModel.applyAnnotations(fileAnnotations, measurementsPerSec)
        }
        onPositionChanged: {
            // Синхронизируем нашу переменную с плеером, когда он сам играет
            currentPosMs = playerController.position
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
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom }

        function seekToMs(ms) {
            if (ms < 0) ms = 0
            currentPosMs = ms // Моментально обновляем позицию, чтобы удаление работало сразу!

            var wasPlaying = playerController.isPlaying
            playerController.play(ms)
            if (!wasPlaying) playerController.pause()
        }

        Item {
            id: waveformContainer
            anchors { left: parent.left; right: parent.right; top: parent.top; bottom: timePassed.top; margins: Theme.paddingLarge }
            visible: filePath !== "" && !playerController.isDecoding
            clip: true

            Item {
                width: waveformList.contentWidth
                height: parent.height
                x: -waveformList.contentX
                z: 1

                Repeater {
                    model: page.annotationsWithIds

                    Item {
                        x: (modelData.t1 / 1000.0) * pxPerSecond
                        width: ((modelData.t2 - modelData.t1) / 1000.0) * pxPerSecond
                        height: parent.height

                        Rectangle { anchors.left: parent.left; width: 2; height: parent.height; color: getColorForType(modelData.type); opacity: 0.8 }
                        Rectangle { anchors.right: parent.right; width: 2; height: parent.height; color: getColorForType(modelData.type); opacity: 0.8 }

                        Item {
                            visible: modelData.type === 1
                            anchors { left: parent.left; bottom: parent.bottom; bottomMargin: Theme.paddingSmall }
                            width: Math.min(240, waveformContainer.width)
                            height: Theme.itemSizeSmall
                            property bool editing: false

                            Rectangle { anchors.fill: parent; radius: 6; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }

                            Label {
                                anchors.fill: parent
                                anchors.leftMargin: Theme.paddingSmall; anchors.rightMargin: Theme.paddingSmall
                                verticalAlignment: Text.AlignVCenter
                                text: page.titleForVoiceId(modelData.voiceId)
                                font.pixelSize: Theme.fontSizeExtraSmall
                                color: Theme.primaryColor
                                elide: Text.ElideRight
                                visible: !parent.editing
                            }

                            TextField {
                                id: editField
                                anchors.fill: parent
                                anchors.leftMargin: Theme.paddingSmall; anchors.rightMargin: Theme.paddingSmall
                                text: page.titleForVoiceId(modelData.voiceId)
                                font.pixelSize: Theme.fontSizeExtraSmall
                                visible: parent.editing

                                onActiveFocusChanged: {
                                    if (!activeFocus && parent.editing) { page.setTitleForVoiceId(modelData.voiceId, text); parent.editing = false }
                                }
                                EnterKey.onClicked: { page.setTitleForVoiceId(modelData.voiceId, text); parent.editing = false }
                            }

                            MouseArea {
                                anchors.fill: parent; enabled: !parent.editing
                                onClicked: { parent.editing = true; editField.forceActiveFocus(); editField.selectAll() }
                            }
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
                z: 0

                Connections {
                    target: playerController
                    onPositionChanged: {
                        if (!playerController.isPlaying) return
                        var xTime = (playerController.position / 1000.0) * pxPerSecond
                        var targetContentX = xTime - waveformList.width / 2
                        var maxX = Math.max(0, waveformList.contentWidth - waveformList.width)

                        if (targetContentX < 0) targetContentX = 0
                        if (targetContentX > maxX) targetContentX = maxX
                        waveformList.contentX = targetContentX
                    }
                }

                delegate: Item {
                    width: barWidthPx
                    height: waveformList.height

                    Rectangle {
                        anchors.centerIn: parent
                        width: 4
                        height: Math.max(4, model.value * parent.height)
                        radius: 2
                        color: getColorForType(model.annotationType)
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var ms = (index / measurementsPerSec) * 1000.0
                            pageRoot.seekToMs(ms)
                        }
                    }
                }
            }

            Rectangle {
                width: 2
                height: parent.height
                color: "red"
                z: 5
                x: ((currentPosMs / 1000.0) * pxPerSecond) - waveformList.contentX
                visible: x > 0 && x < parent.width
            }
        }

        BusyIndicator { anchors.centerIn: waveformContainer; size: BusyIndicatorSize.Large; running: playerController.isDecoding; visible: playerController.isDecoding }

        Label {
            id: timePassed
            anchors { bottom: controlsRow.top; horizontalCenter: parent.horizontalCenter; margins: Theme.horizontalPageMargin }
            text: playerController.pointerPositionToString(currentPosMs)
            font.pixelSize: Theme.fontSizeMedium * 2
            visible: filePath !== "" && !playerController.isDecoding
        }

        Row {
            id: controlsRow
            spacing: Theme.paddingLarge
            anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; margins: Theme.paddingLarge }

            IconButton { icon.source: "image://theme/icon-m-clear"; enabled: filePath !== "" && !playerController.isDecoding; onClicked: deleteSegmentUnderCursor() }
            IconButton { icon.source: "image://theme/icon-m-previous"; enabled: filePath !== "" && !playerController.isDecoding; onClicked: seekToPrevSegment() }

            IconButton {
                icon {
                    source: playerController.isPlaying ? "image://theme/icon-m-pause" : "image://theme/icon-m-simple-play"
                    width: Theme.iconSizeLarge
                    height: Theme.iconSizeLarge
                }
                height: icon.height
                width: icon.width
                enabled: playerController.isPlaybackAvailable && filePath !== "" && !playerController.isDecoding
                onClicked: {
                    if (playerController.isPlaying) playerController.pause()
                    else playerController.play(currentPosMs)
                }
            }

            IconButton { icon.source: "image://theme/icon-m-next"; enabled: filePath !== "" && !playerController.isDecoding; onClicked: seekToNextSegment() }
            IconButton { icon.source: "image://theme/icon-m-delete"; enabled: filePath !== "" && !playerController.isDecoding; onClicked: removeSilenceNow() }
        }

        Label {
            anchors { left: parent.left; right: parent.right; bottom: timePassed.top; margins: Theme.horizontalPageMargin }
            text: lastError
            visible: lastError.length > 0
            color: "red"
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
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
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", { nameFilters: ["*.wav", "*.mp3", "*.ogg", "*.flac"], title: "Выберите аудиофайл" })
        dialog.selectedContentPropertiesChanged.connect(function() {
            if (dialog.selectedContentProperties && dialog.selectedContentProperties.filePath) {
                var selectedPath = dialog.selectedContentProperties.filePath
                filePath = selectedPath
                header.headerText = dialog.selectedContentProperties.fileName || selectedPath.split('/').pop()
                lastError = ""
                voiceLabels = []
                rebuildVoiceIdsAndTitles()
                playerController.setSource(selectedPath)
                pageStack.pop()
            }
        })
    }

    function getColorForType(typeCode) {
        switch (typeCode) {
            case 1: return "lime"
            case 2: return "lightsteelblue"
            case 3: return "red"
            default: return Theme.primaryColor
        }
    }
}
