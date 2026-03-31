#include <QtQuick>
#include <QQmlContext>
#include <auroraapp.h>
// +++ добавь:

#include "./backend/audiosegmenter.h"
#include "./backend/segmentlistmodel.h"

//#include "./backend/sessionmanager.h"
//#include "./backend/sessionmanager.h"
//#include "audiorecordercontroller.h"

#include "./backend/audioplayercontrollerPlayer.h"
int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> application(Aurora::Application::application(argc, argv));
    application->setOrganizationName(QStringLiteral("ru.template"));
    application->setApplicationName(QStringLiteral("smart"));

    //qmlRegisterType<SessionManager>("com.example.sessions", 1, 0, "SessionManager");
    qmlRegisterType<AudioPlayerController>("ru.auroraos.AudioRecorder", 1, 0,
                                           "AudioPlayerController");
    // +++ создаём model + segmenter в C++:
    SegmentListModel segmentModel;
    AudioSegmenter segmenter;
    segmenter.setSegmentModel(&segmentModel);


    //qmlRegisterType<AudioRecorderController>("ru.auroraos.AudioRecorder", 1, 0,
    //                                         "AudioRecorderController");
    //qmlRegisterType<AudioPlayerController>("ru.auroraos.AudioRecorder", 1, 0,
    //                                       "AudioPlayerController");

    QScopedPointer<QQuickView> view(Aurora::Application::createView());

    // +++ пробрасываем в QML:
    view->rootContext()->setContextProperty("segmenter", &segmenter);
    view->rootContext()->setContextProperty("segmentModel", &segmentModel);
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/smart.qml")));
    view->show();

    return application->exec();
}
