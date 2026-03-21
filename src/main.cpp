#include <QtQuick>
#include <auroraapp.h>
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

    //qmlRegisterType<AudioRecorderController>("ru.auroraos.AudioRecorder", 1, 0,
    //                                         "AudioRecorderController");
    //qmlRegisterType<AudioPlayerController>("ru.auroraos.AudioRecorder", 1, 0,
    //                                       "AudioPlayerController");

    QScopedPointer<QQuickView> view(Aurora::Application::createView());
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/smart.qml")));
    view->show();

    return application->exec();
}
