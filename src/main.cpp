#include <QtQuick>
#include <auroraapp.h>

#include "./backend/audioAnalyzer.h"
#include "audioplayercontroller.h"
#include "audioplayercontrollerPlayer.h"
#include "audiorecordercontroller.h"
#include "silenceservice.h"
#include "ApiService.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> application(Aurora::Application::application(argc, argv));
    application->setOrganizationName(QStringLiteral("ru.auroraos"));
    application->setApplicationName(QStringLiteral("SmartVoiceRecorder"));

    qmlRegisterType<AudioPlayerControllerRedact>("ru.auroraos.AudioRecorder",
                                                 1,
                                                 0,
                                                 "AudioPlayerControllerRedact");

    qmlRegisterType<SilenceService>("ru.auroraos.AudioRecorder", 1, 0, "SilenceService");

    qmlRegisterType<AudioRecorderController>("ru.auroraos.AudioRecorder",
                                             1,
                                             0,
                                             "AudioRecorderController");
    qmlRegisterType<AudioPlayerController>("ru.auroraos.AudioRecorder",
                                           1,
                                           0,
                                           "AudioPlayerController");
    qmlRegisterType<AudioAnalyzer>("ru.auroraos.AudioAnalyzer", 1, 0, "AudioAnalyzer");

    ApiService apiService;

    QScopedPointer<QQuickView> view(Aurora::Application::createView());
    view->rootContext()->setContextProperty("apiService", &apiService);
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/SmartVoiceRecorder.qml")));
    view->show();
    return application->exec();
}
