// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_BUFFER_H_
#define PULSE_OPENGL_BUFFER_H_

#include <Pulse.h>
#include "OpenGL.h"

typedef struct OpenGLBuffer
{
} OpenGLBuffer;

PulseBuffer OpenGLCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos);
bool OpenGLMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data);
void OpenGLUnmapBuffer(PulseBuffer buffer);
bool OpenGLCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst);
bool OpenGLCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst);
void OpenGLDestroyBuffer(PulseDevice device, PulseBuffer buffer);

#endif // PULSE_OPENGL_BUFFER_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
