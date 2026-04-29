#ifndef AUDIOBUFFEREXTENSION_H
#define AUDIOBUFFEREXTENSION_H

#include "qglobal.h"
#include <QAudioBuffer>

namespace AudioBufferExtension {
qreal calculateAmplitude(QAudioBuffer buffer);
}

#endif
