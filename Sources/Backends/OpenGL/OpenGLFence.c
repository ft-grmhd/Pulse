// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLFence.h"

PulseFence OpenGLCreateFence(PulseDevice device)
{
}

void OpenGLDestroyFence(PulseDevice device, PulseFence fence)
{
}

bool OpenGLIsFenceReady(PulseDevice device, PulseFence fence)
{
}

bool OpenGLWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
}
