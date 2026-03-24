TARGET = ru.template.smart

CONFIG += \
    auroraapp

QT += core network qml gui quick multimedia

PKGCONFIG += \

SOURCES += \
    backend/amplitudemodelfillerPlayer.cpp \
    backend/audioamplitudePlayer.cpp \
    backend/audioamplitudemodelPlayer.cpp \
    backend/audiobufferextensionPlayer.cpp \
    backend/audioplayercontrollerPlayer.cpp \
    backend/audiorecorderPlayer.cpp \
    backend/sessionmanag.cpp \
    backend/silenceremover.cpp \
    backend/silenceremover_qt.cpp \
    backend/silenceservice.cpp \
    backend/timelineblockPlayer.cpp \
    backend/timelinemodelPlayer.cpp \
    src/main.cpp \

HEADERS += \
    backend/amplitudemodelfillerPlayer.h \
    backend/audioamplitudePlayer.h \
    backend/audioamplitudemodelPlayer.h \
    backend/audiobufferextensionPlayer.h \
    backend/audioplayercontrollerPlayer.h \
    backend/audiorecorderPlayer.h \
    backend/sessionmanag.h \
    backend/silenceremover.h \
    backend/silenceremover_qt.h \
    backend/silenceservice.h \
    backend/timelineblockPlayer.h \
    backend/timelinemodelPlayer.h

DISTFILES += \
    qml/components/RecordTrack.qml \
    qml/pages/AudioFileListPage.qml \
    qml/pages/RecordingPage.qml \
    qml/pages/RedactingPage.qml \
    rpm/ru.template.smart.spec \

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += auroraapp_i18n

TRANSLATIONS += \
    translations/ru.template.smart.ts \
    translations/ru.template.smart-ru.ts \

RESOURCES += \
    resources.qrc
