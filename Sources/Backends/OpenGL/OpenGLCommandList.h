// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_COMMAND_LIST_H_
#define PULSE_OPENGL_COMMAND_LIST_H_

#include <Pulse.h>
#include "OpenGL.h"
#include "OpenGLBuffer.h"

typedef struct OpenGLCommandList
{
} OpenGLCommandList;

PulseCommandList OpenGLRequestCommandList(PulseDevice device, PulseCommandListUsage usage);
bool OpenGLSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
void OpenGLReleaseCommandList(PulseDevice device, PulseCommandList cmd);

#endif // PULSE_OPENGL_COMMAND_LIST_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
