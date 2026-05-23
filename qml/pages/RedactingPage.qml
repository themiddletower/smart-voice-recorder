import QtQuick 2.0
import Sailfish.Silica 1.0
import Aurora.Controls 1.0
import Sailfish.Pickers 1.0
import ru.auroraos.AudioRecorder 1.0
import ru.auroraos.AudioAnalyzer 1.0
import "../components"

Page {
    id: page
    objectName: "mainPage"

    readonly property int silence_type: 2
    readonly property int measurementsPerSec: 10
    readonly property int barWidthPx: 18
    readonly property int pxPerSecond: measurementsPerSec * barWidthPx

    property string filePath: ""
    property alias fileName: header.headerText

    property bool isModified: false
    property string originalFileName: ""
    // -------------------------------

    property real currentPosMs: 0
    property var fileAnnotations: []
    property var voiceLabels: []
    property var annotationsWithIds: []
    property string lastError: ""

    backNavigation: false

    RemorsePopup { id: remorse }

    SilenceService { id: silenceService }
    AudioAnalyzer { id: audioAnalyzer }

    // --- Диалог переименования ---
    Component {
        id: saveAsDialog
        Dialog {
            property alias fileName: nameField.text
            Column {
                width: parent.width; spacing: Theme.paddingMedium
                DialogHeader { title: "Сохранить как" }
                TextField {
                    id: nameField
                    width: parent.width - Theme.horizontalPageMargin * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    label: "Имя файла"
                    placeholderText: "Введите название"
                    EnterKey.onClicked: accept()
                }
            }
        }
    }
/*
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
*/
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

        // Если voiceLabels пустой (первый запуск), создаем стандартные
        // Если НЕ пустой (загружен из JSON), оставляем как есть, только добавляем новые если сегментов стало больше
        var newLabels = []
        for (var id = 1; id <= voiceCounter; id++) {
            var found = null
            for (var j = 0; j < voiceLabels.length; j++) {
                if (voiceLabels[j].voiceId === id) {
                    found = voiceLabels[j];
                    break;
                }
            }
            if (found) {
                newLabels.push(found)
            } else {
                newLabels.push({ "voiceId": id, "title": "человеческая речь " + id })
            }
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
        playerControllerRedact.pause()
        var r = silenceService.removeSilence(filePath, fileAnnotations, silence_type)
        processCutResult(r)
    }

    function deleteSegmentUnderCursor() {
        if (!filePath || filePath === "") return
        var pos = currentPosMs
        var targetIndex = -1

        for (var i = 0; i < fileAnnotations.length; i++) {
            if (pos >= fileAnnotations[i].t1 && pos < fileAnnotations[i].t2) {
                targetIndex = i
                break
            }
        }

        if (targetIndex === -1) {
            lastError = "Нет сегмента под курсором"
            return
        }

        // --- ЛОГИКА СОХРАНЕНИЯ ЗАГОЛОВКОВ ---
        // Если удаляем речь (тип 1), нужно удалить соответствующий заголовок,
        // чтобы остальные заголовки не "съехали"
        if (fileAnnotations[targetIndex].type === 1) {
            var speechRank = 0;
            for (var j = 0; j <= targetIndex; j++) {
                if (fileAnnotations[j].type === 1) speechRank++;
            }
            // Удаляем заголовок из базы и сдвигаем ID у остальных
            var newVoiceLabels = [];
            for (var k = 0; k < voiceLabels.length; k++) {
                if (voiceLabels[k].voiceId < speechRank) {
                    newVoiceLabels.push(voiceLabels[k]);
                } else if (voiceLabels[k].voiceId > speechRank) {
                    var item = voiceLabels[k];
                    item.voiceId--; // Сдвигаем ID вниз
                    newVoiceLabels.push(item);
                }
            }
            voiceLabels = newVoiceLabels;
        }
        // ------------------------------------

        lastError = ""
        playerControllerRedact.pause()

        var fakeAnnotations = JSON.parse(JSON.stringify(fileAnnotations))
        fakeAnnotations[targetIndex].type = 999 // Временный тип для удаления ОДНОГО сегмента

        var r = silenceService.removeSilence(filePath, fakeAnnotations, 999)
        processCutResult(r)
    }

    function processCutResult(r) {
        if (!r || r.error) {
            lastError = r ? r.error : "Ошибка"
            return
        }
        var clean = []
        for (var i = 0; i < r.annotations.length; i++) {
            if (r.annotations[i].t1 < r.annotations[i].t2) clean.push(r.annotations[i])
        }

        filePath = r.outputPath
        isModified = true // Появится кнопка сохранения
        header.headerText = filePath.split('/').pop()

        fileAnnotations = clean
        rebuildVoiceIdsAndTitles()

        playerControllerRedact.setSource(filePath)
        playerControllerRedact.audioAmplitudeModel.applyAnnotations(fileAnnotations, measurementsPerSec)
    }
/*
    function finalizeSave() {
        var dialog = pageStack.push(saveAsDialog, { "fileName": originalFileName.replace(".wav", "") + "_new" })
        dialog.accepted.connect(function() {
            if (silenceService.finalizeSave(filePath, dialog.fileName)) {
                isModified = false
                lastError = "Файл успешно сохранен!"
            } else {
                lastError = "Ошибка при сохранении"
            }
        })
    }
*/
/*
    function finalizeSave() {
        var dialog = pageStack.push(saveAsDialog, {
            "fileName": originalFileName.replace(".wav", "") + "_new"
        })

        dialog.accepted.connect(function() {
            var finalName = dialog.fileName
            // Сохраняем сам аудиофайл
            if (silenceService.finalizeSave(filePath, finalName)) {

                // Формируем путь к новому сохраненному файлу
                var musicDir = StandardPaths.writableLocation(StandardPaths.MusicLocation)
                var finalAudioPath = musicDir + "/" + (finalName.endsWith(".wav") ? finalName : finalName + ".wav")

                // 2. Сохраняем метаданные рядом с новым файлом
                silenceService.saveMetadata(finalAudioPath, fileAnnotations, voiceLabels)

                isModified = false
                lastError = "Файл и метки сохранены!"
            } else {
                lastError = "Ошибка при сохранении"
            }
        })
    }*/

    function seekToNextSegment() {
        var currentPos = currentPosMs
        var nextPos = -1
        for (var i = 0; i < fileAnnotations.length; i++) {
            if (fileAnnotations[i].t1 > currentPos + 50) {
                if (nextPos === -1 || fileAnnotations[i].t1 < nextPos) nextPos = fileAnnotations[i].t1
            }
        }
        if (nextPos !== -1) pageRoot.seekToMs(nextPos)
    }

    function seekToPrevSegment() {
        var currentPos = currentPosMs
        var prevPos = -1
        for (var i = 0; i < fileAnnotations.length; i++) {
            if (fileAnnotations[i].t1 < currentPos - 50) {
                if (prevPos === -1 || fileAnnotations[i].t1 > prevPos) prevPos = fileAnnotations[i].t1
            }
        }
        if (prevPos !== -1) pageRoot.seekToMs(prevPos)
        else pageRoot.seekToMs(0)
    }

/*
    function finalizeSave() {
        var dialog = pageStack.push(saveAsDialog, {
            "fileName": originalFileName.replace(".wav", "") + "_new"
        })

        dialog.accepted.connect(function() {
            var newName = dialog.fileName.trim()
            if (!newName) return

            if (!newName.toLowerCase().endsWith(".wav"))
                newName += ".wav"

            // 1. Сохраняем сам аудиофайл
            var saved = silenceService.finalizeSave(filePath, newName)
            if (!saved) {
                lastError = "Не удалось сохранить аудиофайл"
                return
            }

            // 2. Строим ПРАВИЛЬНЫЙ путь к сохранённому файлу
            var musicDir = StandardPaths.writableLocation(StandardPaths.MusicLocation)
            var finalAudioPath = musicDir + "/" + newName

            console.log("Сохраняем метаданные в:", finalAudioPath + ".json")

            // 3. Сохраняем И аннотации, И названия голосов
            var metaSaved = silenceService.saveMetadata(
                finalAudioPath,
                fileAnnotations,
                voiceLabels
            )

            if (metaSaved) {
                isModified = false
                lastError = "Файл и метки успешно сохранены"
                filePath = finalAudioPath          // обновляем текущий путь
                header.headerText = newName
            } else {
                lastError = "Файл сохранён, но метки не записались"
            }
        })
    }
*/
    function finalizeSave() {
        var dialog = pageStack.push(saveAsDialog, {
            "fileName": originalFileName.replace(".wav", "").replace(".mp3", "") + "_new"
        })

        dialog.accepted.connect(function() {
            var newName = dialog.fileName.trim()
            if (newName === "") return

            // Вместо endsWith используем проверку расширения через регулярку или поиск подстроки
            var hasWavExtension = newName.toLowerCase().indexOf(".wav") !== -1;
            var nameWithExt = hasWavExtension ? newName : newName + ".wav";

            // 1. Сохраняем аудио
            var saved = silenceService.finalizeSave(filePath, nameWithExt)
            if (!saved) {
                lastError = "Ошибка копирования аудио"
                return
            }

            // 2. Получаем чистый путь к папке музыки
            var musicDir = silenceService.getMusicPath()
            var finalAudioPath = musicDir + "/" + nameWithExt

            // 3. Сохраняем метаданные (ВАЖНО: проверяем, что данные есть)
            console.log("!!! Сохраняю метаданные. Кол-во аннотаций: " + fileAnnotations.length)

            var metaSaved = silenceService.saveMetadata(finalAudioPath, fileAnnotations, voiceLabels)

            if (metaSaved) {
                isModified = false
                lastError = "Успешно сохранено!"
                filePath = finalAudioPath
                header.headerText = nameWithExt
            } else {
                lastError = "Аудио сохранено, но метки — нет"
            }
        })
    }

    Component.onCompleted: {
        playerControllerRedact.isPlayerPage = true
    }

    onFileAnnotationsChanged: rebuildVoiceIdsAndTitles()

    Connections {
        target: playerControllerRedact
        onDecodingCompleted: playerControllerRedact.audioAmplitudeModel.applyAnnotations(fileAnnotations, measurementsPerSec)
        onPositionChanged: currentPosMs = playerControllerRedact.position
    }

    AppBar {
            id: header
            // Удаляем свойство headerText, чтобы вручную разместить заголовок по центру

            // 1. Кнопка НАЗАД (Слева)
            IconButton {
                id: backButton
                icon.source: "image://theme/icon-m-back"
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    if (isModified) {
                        askConfirmation("Есть несохраненные изменения. Вы уверены, что хотите выйти?", function() {
                            pageStack.navigateBack(PageStackAction.Animated)
                        })
                    } else {
                        pageStack.navigateBack(PageStackAction.Animated)
                    }
                }
            }

            // 2. ЗАГОЛОВОК (По центру)
            Label {
                text: filePath ? filePath.split('/').pop() : "Выберите аудиофайл"
                anchors.left: backButton.right
                anchors.right: rightButtons.left
                anchors.leftMargin: Theme.paddingMedium
                anchors.rightMargin: Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight // Если имя слишком длинное, оно сократится тремя точками
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.primaryColor
            }

            // 3. БЛОК КНОПОК СПРАВА (Облако, Сохранить, Папка)
            Row {
                id: rightButtons
                anchors.right: parent.right
                anchors.rightMargin: Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                spacing: Theme.paddingSmall

                IconButton {
                    icon.source: "image://theme/icon-m-cloud-upload"
                    visible: filePath !== ""
                    onClicked: pageStack.push(Qt.resolvedUrl("CloudStoragePage.qml"), {
                        "currentLocalFile": page.filePath,
                        "currentAnnotations": JSON.stringify(page.fileAnnotations),
                        "currentLabels": JSON.stringify(page.voiceLabels)
                    })
                }

                IconButton {
                    icon.source: "image://theme/icon-m-save"
                    visible: isModified
                    onClicked: finalizeSave()
                }

                IconButton {
                    icon.source: "image://theme/icon-m-folder"
                    onClicked: openFilePicker()
                }
            }
        }

    Item {
        id: pageRoot
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom }

        function seekToMs(ms) {
            if (ms < 0) ms = 0
            currentPosMs = ms
            var wasPlaying = playerControllerRedact.isPlaying
            playerControllerRedact.play(ms)
            if (!wasPlaying) playerControllerRedact.pause()
        }

        Item {
            id: waveformContainer
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; verticalCenterOffset: -Theme.paddingLarge * 2 }
            height: parent.height * 0.45
            visible: filePath !== "" && !playerControllerRedact.isDecoding
            clip: true

            Item {
                width: waveformList.contentWidth; height: parent.height; x: -waveformList.contentX; z: 1
                Repeater {
                    model: page.annotationsWithIds
                    Item {
                        x: (modelData.t1 / 1000.0) * pxPerSecond
                        width: ((modelData.t2 - modelData.t1) / 1000.0) * pxPerSecond
                        height: parent.height
                        Item {
                            visible: modelData.type === 1
                            anchors { left: parent.left; bottom: parent.bottom; bottomMargin: Theme.paddingSmall }
                            width: Math.min(240, waveformContainer.width)
                            height: parent.editing ? Theme.itemSizeSmall : Math.max(Theme.itemSizeSmall, segmentLabel.implicitHeight + 16)
                            property bool editing: false
                            Rectangle { anchors.fill: parent; radius: 6; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
                            Label {
                                id: segmentLabel; anchors.fill: parent; anchors.leftMargin: Theme.paddingSmall; anchors.rightMargin: Theme.paddingSmall
                                verticalAlignment: Text.AlignVCenter; text: page.titleForVoiceId(modelData.voiceId)
                                font.pixelSize: Theme.fontSizeExtraSmall; color: Theme.primaryColor; wrapMode: Text.Wrap; visible: !parent.editing
                            }
                            TextField {
                                id: editField; anchors.fill: parent; anchors.leftMargin: Theme.paddingSmall; anchors.rightMargin: Theme.paddingSmall
                                text: page.titleForVoiceId(modelData.voiceId); font.pixelSize: Theme.fontSizeExtraSmall; visible: parent.editing
                                onActiveFocusChanged: if (!activeFocus && parent.editing) { page.setTitleForVoiceId(modelData.voiceId, text); parent.editing = false }
                                EnterKey.onClicked: { page.setTitleForVoiceId(modelData.voiceId, text); parent.editing = false }
                            }
                            MouseArea { anchors.fill: parent; enabled: !parent.editing; onClicked: { parent.editing = true; editField.forceActiveFocus(); editField.selectAll() } }
                        }
                    }
                }
            }

            ListView {
                id: waveformList; anchors.fill: parent; orientation: ListView.Horizontal
                model: playerControllerRedact.audioAmplitudeModel; boundsBehavior: Flickable.StopAtBounds; z: 0
                Connections {
                    target: playerControllerRedact
                    onPositionChanged: {
                        if (!playerControllerRedact.isPlaying) return
                        var xTime = (playerControllerRedact.position / 1000.0) * pxPerSecond
                        var targetContentX = xTime - waveformList.width / 2
                        var maxX = Math.max(0, waveformList.contentWidth - waveformList.width)
                        if (targetContentX < 0) targetContentX = 0
                        if (targetContentX > maxX) targetContentX = maxX
                        waveformList.contentX = targetContentX
                    }
                }
                delegate: Item {
                    width: barWidthPx; height: waveformList.height
                    Rectangle {
                        anchors.centerIn: parent; width: 12
                        height: Math.max(4, (model.value * parent.height) / 2); radius: 4
                        color: getColorForType(model.annotationType)
                    }
                    MouseArea { anchors.fill: parent; onClicked: { var ms = (index / measurementsPerSec) * 1000.0; pageRoot.seekToMs(ms) } }
                }
            }

            Rectangle { width: 2; height: parent.height; color: "red"; z: 5; x: ((currentPosMs / 1000.0) * pxPerSecond) - waveformList.contentX; visible: x > 0 && x < parent.width }
        }

        BusyIndicator { anchors.centerIn: waveformContainer; size: BusyIndicatorSize.Large; running: playerControllerRedact.isDecoding; visible: playerControllerRedact.isDecoding }

        Label {
            id: timePassed; anchors { bottom: controlsRow.top; horizontalCenter: parent.horizontalCenter; margins: Theme.horizontalPageMargin }
            text: playerControllerRedact.pointerPositionToString(currentPosMs); font.pixelSize: Theme.fontSizeMedium * 2; visible: filePath !== "" && !playerControllerRedact.isDecoding
        }

        Row {
            id: controlsRow; spacing: Theme.paddingLarge; anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; margins: Theme.paddingLarge }
            IconButton {
                    icon.source: "image://theme/icon-m-clear"
                    enabled: filePath !== "" && !playerControllerRedact.isDecoding
                    onClicked: {
                        // "Удаление" начнется через 3 секунды. Если нажать на полоску — отменится.
                        remorse.execute("Удаление фрагмента", function() {
                            deleteSegmentUnderCursor()
                        })
                    }
                }
            IconButton { icon.source: "image://theme/icon-m-previous"; enabled: filePath !== "" && !playerControllerRedact.isDecoding; onClicked: seekToPrevSegment() }
            IconButton {
                icon { source: playerControllerRedact.isPlaying ? "image://theme/icon-m-pause" : "image://theme/icon-m-simple-play"; width: Theme.iconSizeLarge; height: Theme.iconSizeLarge }
                height: icon.height; width: icon.width; enabled: playerControllerRedact.isPlaybackAvailable && filePath !== "" && !playerControllerRedact.isDecoding
                onClicked: if (playerControllerRedact.isPlaying) playerControllerRedact.pause(); else playerControllerRedact.play(currentPosMs)
            }
            IconButton { icon.source: "image://theme/icon-m-next"; enabled: filePath !== "" && !playerControllerRedact.isDecoding; onClicked: seekToNextSegment() }
            IconButton {
                    icon.source: "image://theme/icon-m-delete"
                    enabled: filePath !== "" && !playerControllerRedact.isDecoding
                    onClicked: {
                        askConfirmation("Вы уверены, что хотите вырезать ВСЮ тишину из аудио?", function() {
                            removeSilenceNow()
                        })
                    }
                }
        }

        Label {
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: timePassed.top
                        margins: Theme.horizontalPageMargin
                    }
                    text: lastError
                    visible: lastError.length > 0
                    color: "red"
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall
                }
        Label { anchors.centerIn: parent; text: "Выберите аудиофайл\nнажмите на иконку папки вверху"; horizontalAlignment: Text.AlignHCenter; color: Theme.secondaryColor; visible: filePath === ""; font.pixelSize: Theme.fontSizeMedium }
    }
/*
    function openFilePicker() {
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", { nameFilters: ["*.wav", "*.mp3", "*.ogg", "*.flac"], title: "Выберите аудиофайл" })
        dialog.selectedContentPropertiesChanged.connect(function() {
            if (dialog.selectedContentProperties && dialog.selectedContentProperties.filePath) {
                var selectedPath = dialog.selectedContentProperties.filePath
                filePath = selectedPath
                originalFileName = dialog.selectedContentProperties.fileName || selectedPath.split('/').pop()
                isModified = false
                header.headerText = originalFileName
                lastError = ""
                voiceLabels = []
                rebuildVoiceIdsAndTitles()
                playerControllerRedact.setSource(selectedPath)
                fileAnnotations = audioAnalyzer.analyzeFile(selectedPath)
                pageStack.pop()
            }
        })
    }
*/
/*
    function openFilePicker() {
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", {
            nameFilters: ["*.wav", "*.mp3"],
            title: "Выберите аудиофайл"
        })

        dialog.selectedContentPropertiesChanged.connect(function() {
            if (dialog.selectedContentProperties && dialog.selectedContentProperties.filePath) {
                var selectedPath = dialog.selectedContentProperties.filePath
                filePath = selectedPath
                originalFileName = dialog.selectedContentProperties.fileName || selectedPath.split('/').pop()

                // 1. Пытаемся загрузить метаданные
                var meta = silenceService.loadMetadata(selectedPath)

                if (meta.annotations && meta.annotations.length > 0) {
                    // Если метаданные найдены — используем их
                    console.log("Загружены сохраненные аннотации")
                    fileAnnotations = meta.annotations
                    voiceLabels = meta.voiceLabels || []
                    lastError = "Загружены сохраненные метки"
                } else {
                    // Если файла нет — запускаем нейронку (старая логика)
                    console.log("Метаданные не найдены, запуск анализатора...")
                    fileAnnotations = audioAnalyzer.analyzeFile(selectedPath)
                    voiceLabels = []
                }

                isModified = false
                header.headerText = originalFileName
                rebuildVoiceIdsAndTitles()
                playerControllerRedact.setSource(selectedPath)
                pageStack.pop()
            }
        })
    }
    */
    /*
    function openFilePicker() {
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", {
            nameFilters: ["*.wav", "*.mp3", "*.ogg", "*.flac"],
            title: "Выберите аудиофайл"
        })

        dialog.selectedContentPropertiesChanged.connect(function() {
            if (!dialog.selectedContentProperties || !dialog.selectedContentProperties.filePath) {
                return;
            }

            var selectedPath = dialog.selectedContentProperties.filePath;
            var fileNameOnly = dialog.selectedContentProperties.fileName || selectedPath.split('/').pop();

            // Сбрасываем текущее состояние
            lastError = "Загрузка...";
            filePath = selectedPath;
            originalFileName = fileNameOnly;
            isModified = false;
            header.headerText = fileNameOnly;

            // --- ЛОГИКА ПОИСКА МЕТАДАННЫХ ---

            // Шаг 1: Пробуем загрузить JSON, который лежит рядом с файлом
            var meta = silenceService.loadMetadata(selectedPath)
            console.log("Попытка загрузить JSON:", selectedPath + ".json")
            console.log("Найдено аннотаций:", meta.annotations ? meta.annotations.length : 0)

            if (meta.annotations && meta.annotations.length > 0) {
                fileAnnotations = meta.annotations
                voiceLabels = meta.voiceLabels || []
                rebuildVoiceIdsAndTitles()
                lastError = "Метки загружены из файла"
            }
            else {
                // Шаг 2: JSON рядом нет. Пробуем найти по хэшу (умное восстановление)
                console.log("[Metadata] JSON не найден, проверяем базу бэкапов по хэшу...");
                var fileHash = silenceService.getFileHash(selectedPath);
                var backupPath = silenceService.getAppDataPath() + "/" + fileHash + ".json";

                var backupMeta = silenceService.loadMetadataFromFile(backupPath);

                if (backupMeta.annotations && backupMeta.annotations.length > 0) {
                    console.log("[Metadata] Ура! Нашли бэкап меток по хэшу звука");
                    fileAnnotations = backupMeta.annotations;
                    voiceLabels = backupMeta.voiceLabels || [];
                    rebuildVoiceIdsAndTitles()          // ← обязательно!

                    // Шаг 3: "Самоисцеление" - сохраняем JSON рядом с аудио для будущего использования
                    silenceService.saveMetadata(selectedPath, fileAnnotations, voiceLabels);
                    lastError = "Связь с метками восстановлена по хэшу";
                }
                else {
                    // Шаг 4: Ничего не нашли - запускаем нейросеть
                    console.log("[Metadata] Данных нет нигде. Запуск анализатора...");
                    fileAnnotations = audioAnalyzer.analyzeFile(selectedPath);
                    voiceLabels = [];
                    lastError = "Метки созданы нейросетью";

                    // Сразу сохраним результат в бэкап по хэшу, чтобы не анализировать снова
                    silenceService.saveMetadata(selectedPath, fileAnnotations, voiceLabels);
                }
            }

            // --- ОБНОВЛЕНИЕ ИНТЕРФЕЙСА ---
            rebuildVoiceIdsAndTitles();
            playerControllerRedact.setSource(selectedPath);

            // Закрываем пикер
            pageStack.pop();
        })
    }
    */
/*
    function openFilePicker() {
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", {
            nameFilters: ["*.wav", "*.mp3", "*.ogg", "*.flac"],
            title: "Выберите аудиофайл"
        })

        dialog.selectedContentPropertiesChanged.connect(function() {
            if (!dialog.selectedContentProperties || !dialog.selectedContentProperties.filePath) {
                return
            }

            var selectedPath = dialog.selectedContentProperties.filePath
            var fileNameOnly = dialog.selectedContentProperties.fileName || selectedPath.split('/').pop()

            lastError = "Загрузка..."
            filePath = selectedPath
            originalFileName = fileNameOnly
            isModified = false
            header.headerText = fileNameOnly

            // Пробуем загрузить метаданные рядом с файлом
            var meta = silenceService.loadMetadata(selectedPath)
            console.log("Проверка JSON:", selectedPath + ".json")
            console.log("Аннотаций найдено:", meta.annotations ? meta.annotations.length : 0)

            if (meta.annotations && meta.annotations.length > 0) {
                fileAnnotations = meta.annotations
                voiceLabels = meta.voiceLabels || []
                rebuildVoiceIdsAndTitles()
                lastError = "Метки загружены из файла"
            } else {
                // Пробуем восстановить по хэшу
                var fileHash = silenceService.getFileHash(selectedPath)
                var backupPath = silenceService.getAppDataPath() + "/" + fileHash + ".json"
                var backupMeta = silenceService.loadMetadataFromFile(backupPath)

                if (backupMeta.annotations && backupMeta.annotations.length > 0) {
                    console.log("Метки восстановлены по хэшу")
                    fileAnnotations = backupMeta.annotations
                    voiceLabels = backupMeta.voiceLabels || []
                    rebuildVoiceIdsAndTitles()
                    silenceService.saveMetadata(selectedPath, fileAnnotations, voiceLabels)
                    lastError = "Метки восстановлены по хэшу"
                } else {
                    // Запускаем нейросеть
                    console.log("Метаданных нет — запускаем анализатор")
                    fileAnnotations = audioAnalyzer.analyzeFile(selectedPath)
                    voiceLabels = []
                    rebuildVoiceIdsAndTitles()
                    lastError = "Метки созданы нейросетью"
                    // Сохраняем сразу, чтобы в следующий раз не анализировать
                    silenceService.saveMetadata(selectedPath, fileAnnotations, voiceLabels)
                }
            }

            playerControllerRedact.setSource(selectedPath)
            pageStack.pop()
        })
    }*/
    function openFilePicker() {
        var dialog = pageStack.push("Sailfish.Pickers.FilePickerPage", {
            nameFilters: ["*.wav", "*.mp3", "*.ogg", "*.flac"],
            title: "Выберите аудиофайл"
        })

        dialog.selectedContentPropertiesChanged.connect(function() {
            if (!dialog.selectedContentProperties || !dialog.selectedContentProperties.filePath) {
                return;
            }

            var selectedPath = dialog.selectedContentProperties.filePath.toString();
            // Очищаем путь от префикса file:// для корректной работы в C++
            var cleanPath = selectedPath.replace("file://", "");
            var fileNameOnly = dialog.selectedContentProperties.fileName || cleanPath.split('/').pop();

            // Сброс состояния страницы
            lastError = "Загрузка данных...";
            filePath = cleanPath;
            originalFileName = fileNameOnly;
            isModified = false;
            header.headerText = fileNameOnly;
            voiceLabels = []; // Очищаем старые имена перед загрузкой новых

            console.log("[Picker] Выбран файл: " + cleanPath);

            // --- ЛОГИКА ПОИСКА МЕТАДАННЫХ ---

            // 1. Пытаемся загрузить JSON, который лежит прямо рядом с аудиофайлом
            var meta = silenceService.loadMetadata(cleanPath);

            // Проверка наличия данных (совместимо со старым JS)
            var hasValidMeta = meta && meta.annotations && meta.annotations.length > 0;

            if (hasValidMeta) {
                console.log("[Picker] Успех: Метки найдены рядом с файлом");
                fileAnnotations = meta.annotations;
                voiceLabels = meta.voiceLabels || [];
                lastError = "Метки загружены из файла .json";
            }
            else {
                // 2. Если рядом файла нет, ищем в базе бэкапов приложения по хэшу (умное восстановление)
                console.log("[Picker] Рядом с файлом ничего нет. Считаем хэш...");
                var fileHash = silenceService.getFileHash(cleanPath);
                var backupPath = silenceService.getAppDataPath() + "/" + fileHash + ".json";

                console.log("[Picker] Поиск в бэкапах: " + backupPath);
                var backupMeta = silenceService.loadMetadataFromFile(backupPath);

                var hasBackup = backupMeta && backupMeta.annotations && backupMeta.annotations.length > 0;

                if (hasBackup) {
                    console.log("[Picker] Успех: Метки восстановлены из бэкапа по хэшу");
                    fileAnnotations = backupMeta.annotations;
                    voiceLabels = backupMeta.voiceLabels || [];
                    lastError = "Метки восстановлены из базы бэкапов";

                    // Попытка "вылечить" файл - сохранить JSON рядом с аудио, если это не папка /opt/
                    if (cleanPath.indexOf("/opt/") === -1) {
                        silenceService.saveMetadata(cleanPath, fileAnnotations, voiceLabels);
                    }
                }
                else {
                    // 3. Если данных нет нигде — запускаем анализатор (нейросеть)
                    console.log("[Picker] Данных нет. Запуск AudioAnalyzer...");
                    fileAnnotations = audioAnalyzer.analyzeFile(cleanPath);
                    voiceLabels = []; // Сбрасываем названия для нового анализа
                    lastError = "Файл проанализирован нейросетью";

                    // Сразу сохраняем результат в бэкап (AppData), чтобы не анализировать снова при ошибках записи в /opt/
                    silenceService.saveMetadata(cleanPath, fileAnnotations, voiceLabels);
                }
            }

            // --- ОБНОВЛЕНИЕ ИНТЕРФЕЙСА ---
            // Сначала перестраиваем ID и заголовки
            rebuildVoiceIdsAndTitles();

            // Загружаем аудио в плеер
            playerControllerRedact.setSource(cleanPath);

            // Принудительно обновляем визуализацию волновой формы
            if (playerControllerRedact.audioAmplitudeModel) {
                playerControllerRedact.audioAmplitudeModel.applyAnnotations(fileAnnotations, measurementsPerSec);
            }

            //pageStack.pop();
            pageStack.replace(Qt.resolvedUrl("RedactingPage.qml"), {
                "filePath": cleanPath,
                "fileAnnotations": fileAnnotations,
                "voiceLabels": voiceLabels
            });
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

    Component {
            id: confirmDialogComponent
            Dialog {
                property string message: ""
                Column {
                    width: parent.width
                    spacing: Theme.paddingLarge
                    DialogHeader {
                        acceptText: "Да"
                        cancelText: "Нет"
                    }
                    Label {
                        text: message
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        color: Theme.highlightColor
                        wrapMode: Text.Wrap
                        font.pixelSize: Theme.fontSizeMedium
                    }
                }
            }
        }

        // Вспомогательная функция для вызова диалога
        function askConfirmation(text, callback) {
            var dialog = pageStack.push(confirmDialogComponent, { "message": text })
            dialog.accepted.connect(callback)
        }
}
