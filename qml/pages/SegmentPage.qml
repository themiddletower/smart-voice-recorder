// qml/pages/SegmentPage.qml
//
// Единственная страница: выбор файла → сегментация → результат.

import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: "Сегментация аудио" }

            // ═══ Ввод пути к файлу ═══
            TextField {
                id: filePathField
                width: parent.width
                placeholderText: "Путь к WAV-файлу"
                label: "Файл"
                // Для теста можно вписать путь вручную
                text: "/home/defaultuser/Music/test.wav"
            }

            // ═══ Кнопка запуска ═══
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: segmenter.status === "processing"
                      ? "Обработка..." : "Сегментировать"
                enabled: segmenter.status !== "processing"
                onClicked: segmenter.processFile(filePathField.text)
            }

            // ═══ Прогресс ═══
            ProgressBar {
                width: parent.width
                visible: segmenter.status === "processing"
                minimumValue: 0
                maximumValue: 100
                value: segmenter.progress
                label: segmenter.progress + "%"
            }

            // ═══ Ошибка ═══
            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                visible: segmenter.status === "error"
                color: "#F44336"
                wrapMode: Text.Wrap
                text: segmenter.errorMessage
            }

            // ═══ Информация о файле ═══
            Column {
                width: parent.width
                visible: segmenter.status === "done"
                spacing: Theme.paddingSmall

                SectionHeader { text: "Информация" }

                DetailItem {
                    label: "Частота дискретизации"
                    value: segmenter.fileSampleRate + " Гц"
                }
                DetailItem {
                    label: "Длительность"
                    value: (segmenter.fileDurationMs / 1000).toFixed(1) + " сек"
                }
                DetailItem {
                    label: "Всего сегментов"
                    value: segmentModel.count
                }
            }

            // ═══ Статистика по типам ═══
            Column {
                width: parent.width
                visible: segmenter.status === "done"
                spacing: Theme.paddingSmall

                SectionHeader { text: "Статистика" }

                // Речь
                Item {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    height: 40
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        anchors.fill: parent
                        color: "#1B5E20"
                        radius: 5
                        opacity: 0.3
                    }
                    Rectangle {
                        height: parent.height
                        radius: 5
                        color: "#4CAF50"
                        width: {
                            var total = segmentModel.totalDurationMs
                            if (total <= 0) return 0
                            return parent.width *
                                   segmentModel.speechDurationMs / total
                        }
                    }
                    Label {
                        anchors.centerIn: parent
                        font.pixelSize: Theme.fontSizeSmall
                        color: "white"
                        text: "Речь: " +
                              (segmentModel.speechDurationMs / 1000).toFixed(1) +
                              " сек"
                    }
                }

                // Тишина
                Item {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    height: 40
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        anchors.fill: parent
                        color: "#424242"
                        radius: 5
                        opacity: 0.3
                    }
                    Rectangle {
                        height: parent.height
                        radius: 5
                        color: "#888888"
                        width: {
                            var total = segmentModel.totalDurationMs
                            if (total <= 0) return 0
                            return parent.width *
                                   segmentModel.silenceDurationMs / total
                        }
                    }
                    Label {
                        anchors.centerIn: parent
                        font.pixelSize: Theme.fontSizeSmall
                        color: "white"
                        text: "Тишина: " +
                              (segmentModel.silenceDurationMs / 1000).toFixed(1) +
                              " сек"
                    }
                }

                // Шум
                Item {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    height: 40
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        anchors.fill: parent
                        color: "#E65100"
                        radius: 5
                        opacity: 0.3
                    }
                    Rectangle {
                        height: parent.height
                        radius: 5
                        color: "#FF9800"
                        width: {
                            var total = segmentModel.totalDurationMs
                            if (total <= 0) return 0
                            return parent.width *
                                   segmentModel.noiseDurationMs / total
                        }
                    }
                    Label {
                        anchors.centerIn: parent
                        font.pixelSize: Theme.fontSizeSmall
                        color: "white"
                        text: "Шум: " +
                              (segmentModel.noiseDurationMs / 1000).toFixed(1) +
                              " сек"
                    }
                }
            }

            // ═══ Таймлайн ═══
            Column {
                width: parent.width
                visible: segmenter.status === "done" && segmentModel.count > 0

                SectionHeader { text: "Таймлайн" }

                Rectangle {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    height: 40
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#222222"
                    radius: 5
                    clip: true

                    Row {
                        anchors.fill: parent

                        Repeater {
                            model: segmentModel
                            delegate: Rectangle {
                                width: {
                                    var total = segmentModel.totalDurationMs
                                    if (total <= 0) return 0
                                    return Math.max(1,
                                        (durationMs / total) * parent.width)
                                }
                                height: parent.height
                                color: {
                                    switch (segmentType) {
                                    case 0: return "#555555"
                                    case 1: return "#4CAF50"
                                    case 2: return "#FF9800"
                                    default: return "#555555"
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ═══ Список сегментов ═══
            Column {
                width: parent.width
                visible: segmenter.status === "done" && segmentModel.count > 0

                SectionHeader {
                    text: "Сегменты (" + segmentModel.count + ")"
                }

                Repeater {
                    model: segmentModel

                    delegate: ListItem {
                        contentHeight: Theme.itemSizeSmall

                        Row {
                            anchors {
                                left: parent.left
                                leftMargin: Theme.horizontalPageMargin
                                verticalCenter: parent.verticalCenter
                            }
                            spacing: Theme.paddingMedium

                            // Цветная метка типа
                            Rectangle {
                                width: 8
                                height: 32
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: {
                                    switch (segmentType) {
                                    case 0: return "#888888"
                                    case 1: return "#4CAF50"
                                    case 2: return "#FF9800"
                                    default: return "#888888"
                                    }
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter

                                Label {
                                    font.pixelSize: Theme.fontSizeSmall
                                    color: Theme.primaryColor
                                    text: {
                                        var names = [
                                            "Тишина", "Речь", "Шум"
                                        ]
                                        return (index + 1) + ". " +
                                               (names[segmentType] || "?")
                                    }
                                }

                                Label {
                                    font.pixelSize: Theme.fontSizeTiny
                                    color: Theme.secondaryColor
                                    text: {
                                        function fmt(ms) {
                                            var s = Math.floor(ms / 1000)
                                            var m = Math.floor(s / 60)
                                            s = s % 60
                                            var msRem = ms % 1000
                                            return (m < 10 ? "0" : "") + m +
                                                   ":" +
                                                   (s < 10 ? "0" : "") + s +
                                                   "." +
                                                   (msRem < 100 ? "0" : "") +
                                                   Math.floor(msRem / 10)
                                        }
                                        return fmt(startMs) + " → " +
                                               fmt(endMs) +
                                               "  (" +
                                               (durationMs/1000).toFixed(1) +
                                               " с, ур. " +
                                               (avgLevel * 100).toFixed(0) +
                                               "%)"
                                    }
                                }
                            }
                        }
                    }
                }

                Item { width: 1; height: Theme.paddingLarge }
            }
        }
    }
}
