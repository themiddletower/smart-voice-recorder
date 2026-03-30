import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.auroraos.AudioRecorder 1.0

ApplicationWindow {
    objectName: "applicationWindow"
    initialPage: Qt.resolvedUrl("pages/MainPage.qml")
    cover: Qt.resolvedUrl("cover/DefaultCoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    AudioPlayerController {
        id: playerController
    }
}
