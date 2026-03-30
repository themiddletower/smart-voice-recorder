TARGET = ru.template.smart

CONFIG += \
    auroraapp

PKGCONFIG += \

SOURCES += \
    backend/audioamplitudemodel.cpp \
    backend/audiobufferextension.cpp \
    backend/audioplayercontroller.cpp \
    backend/audiorecorder.cpp \
    backend/audiorecordercontroller.cpp \
    backend/sessionmanag.cpp \
    backend/timelinemodel.cpp \
    src/main.cpp \

HEADERS += \
    backend/audioamplitudemodel.h \
    backend/audiobufferextension.h \
    backend/audioplayercontroller.h \
    backend/audiorecorder.h \
    backend/audiorecordercontroller.h \
    backend/sessionmanag.h \
    backend/timelinemodel.h

DISTFILES += \
    qml/pages/DictaphonePage.qml \
    qml/pages/RecordTrack.qml \
    qml/pages/RecordingPage.qml \
    qml/pages/RedactingPage.qml \
    rpm/ru.template.smart.spec \

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += auroraapp_i18n

QT += multimedia
INCLUDEPATH += backend
TRANSLATIONS += \
    translations/ru.template.smart.ts \
    translations/ru.template.smart-ru.ts \
