TARGET = ru.template.smart

# CONFIG += \
#     auroraapp

CONFIG += auroraapp
CONFIG += c++17

QT += core network qml gui quick multimedia

PKGCONFIG += \

INCLUDEPATH += \
    3rdparty/libfvad/include \
    3rdparty/libfvad/src

SOURCES += \
    backend/amplitudemodelfillerPlayer.cpp \
    backend/audioamplitudePlayer.cpp \
    backend/audioamplitudemodelPlayer.cpp \
    backend/audiobufferextensionPlayer.cpp \
    backend/audioplayercontrollerPlayer.cpp \
    backend/audiorecorderPlayer.cpp \
    backend/audiosegmenter.cpp \
    backend/segmentlistmodel.cpp \
    backend/sessionmanag.cpp \
    backend/timelineblockPlayer.cpp \
    backend/timelinemodelPlayer.cpp \
    backend/vadengine.cpp \
    backend/wavreader.cpp \
    src/main.cpp

HEADERS += \
    3rdparty/libfvad/include/fvad.h \
    backend/audiosegment.h \
    backend/audiosegmenter.h \
    backend/segmentlistmodel.h \
    backend/vadengine.h \
    backend/wavreader.h

SOURCES += \
    3rdparty/libfvad/src/fvad.c \
    3rdparty/libfvad/src/vad/vad_core.c \
    3rdparty/libfvad/src/vad/vad_filterbank.c \
    3rdparty/libfvad/src/vad/vad_gmm.c \
    3rdparty/libfvad/src/vad/vad_sp.c \
    3rdparty/libfvad/src/signal_processing/energy.c \
    3rdparty/libfvad/src/signal_processing/get_scaling_square.c \
    3rdparty/libfvad/src/signal_processing/division_operations.c \
    3rdparty/libfvad/src/signal_processing/spl_inl.c \
    3rdparty/libfvad/src/signal_processing/resample_48khz.c \
    3rdparty/libfvad/src/signal_processing/resample_by_2_internal.c \
    3rdparty/libfvad/src/signal_processing/resample_fractional.c

HEADERS += \
    backend/amplitudemodelfillerPlayer.h \
    backend/audioamplitudePlayer.h \
    backend/audioamplitudemodelPlayer.h \
    backend/audiobufferextensionPlayer.h \
    backend/audioplayercontrollerPlayer.h \
    backend/audiorecorderPlayer.h \
    backend/sessionmanag.h \
    backend/timelineblockPlayer.h \
    backend/timelinemodelPlayer.h

DISTFILES += \
    qml/components/RecordTrack.qml \
    qml/pages/AudioFileListPage.qml \
    qml/pages/RecordingPage.qml \
    qml/pages/RedactingPage.qml \
    qml/pages/SegmentPage.qml \
    rpm/ru.template.smart.spec

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += auroraapp_i18n

TRANSLATIONS += \
    translations/ru.template.smart.ts \
    translations/ru.template.smart-ru.ts

RESOURCES += \
    resources.qrc
