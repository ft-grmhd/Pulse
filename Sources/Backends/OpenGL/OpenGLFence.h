// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_FENCE_H_
#define PULSE_OPENGL_FENCE_H_

#include <Pulse.h>
#include "OpenGL.h"

PulseFence OpenGLCreateFence(PulseDevice device);
void OpenGLDestroyFence(PulseDevice device, PulseFence fence);
bool OpenGLIsFenceReady(PulseDevice device, PulseFence fence);
bool OpenGLWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all);

#endif // PULSE_OPENGL_FENCE_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
