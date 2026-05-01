TARGET = ru.template.smart

CONFIG += \
    auroraapp

QT += core network qml gui quick multimedia

PKGCONFIG += \

SOURCES += \
    backend/ApiService.cpp \
    backend/amplitudemodelfillerPlayer.cpp \
    backend/audioAnalyzer.cpp \
    backend/audioamplitudePlayer.cpp \
    backend/audioamplitudemodel.cpp \
    backend/audioamplitudemodelPlayer.cpp \
    backend/audiobufferextension.cpp \
    backend/audiobufferextensionPlayer.cpp \
    backend/audioplayercontroller.cpp \
    backend/audioplayercontrollerPlayer.cpp \
    backend/audiorecorder.cpp \
    backend/audiorecorderPlayer.cpp \
    backend/audiorecordercontroller.cpp \
    backend/sessionmanag.cpp \
    backend/silenceremover.cpp \
    backend/silenceremover_qt.cpp \
    backend/silenceservice.cpp \
    backend/smartDenoiseWavSoft.cpp \
    backend/timelineblockPlayer.cpp \
    backend/timelinemodel.cpp \
    backend/timelinemodelPlayer.cpp \
    src/main.cpp \

HEADERS += \
    backend/ApiService.h \
    backend/amplitudemodelfillerPlayer.h \
    backend/audioAnalyzer.h \
    backend/audioamplitudePlayer.h \
    backend/audioamplitudemodel.h \
    backend/audioamplitudemodelPlayer.h \
    backend/audiobufferextension.h \
    backend/audiobufferextensionPlayer.h \
    backend/audioplayercontroller.h \
    backend/audioplayercontrollerPlayer.h \
    backend/audiorecorder.h \
    backend/audiorecorderPlayer.h \
    backend/audiorecordercontroller.h \
    backend/sessionmanag.h \
    backend/silenceremover.h \
    backend/silenceremover_qt.h \
    backend/silenceservice.h \
    backend/smartDenoiseWavSoft.h \
    backend/timelineblockPlayer.h \
    backend/timelinemodel.h \
    backend/timelinemodelPlayer.h

DISTFILES += \
    qml/components/RecordTrack.qml \
    qml/components/SaveDialog.qml \
    qml/pages/AudioFileListPage.qml \
    qml/pages/DictaphonePage.qml \
    qml/pages/RecordTrack.qml \
    qml/pages/RecordingPage.qml \
    qml/pages/RedactingPage.qml \
    rpm/ru.template.smart.spec \

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += auroraapp_i18n

INCLUDEPATH += backend
TRANSLATIONS += \
    translations/ru.template.smart.ts \
    translations/ru.template.smart-ru.ts \

RESOURCES += \
    resources.qrc

INCLUDEPATH += \
    $$PWD/3rdparty/libfvad/include \
    $$PWD/3rdparty/libfvad/src

SOURCES += $$files($$PWD/3rdparty/libfvad/src/*.c, true)


HEADERS += $$files($$PWD/3rdparty/libfvad/src/*.h, true) \
           $$PWD/3rdparty/libfvad/include/fvad.h
