// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLFence.h"
#include "OpenGLDevice.h"

PulseFence OpenGLCreateFence(PulseDevice device)
{
	PULSE_UNUSED(device);
	PulseFence fence = (PulseFence)calloc(1, sizeof(PulseFence));
	PULSE_CHECK_ALLOCATION_RETVAL(fence, PULSE_NULL_HANDLE);
	return fence;
}

void OpenGLDestroyFence(PulseDevice device, PulseFence fence)
{
	PULSE_UNUSED(device);
	free(fence);
}

bool OpenGLIsFenceReady(PulseDevice device, PulseFence fence)
{
	PULSE_UNUSED(device);
	PULSE_UNUSED(fence);
	return true;
}

bool OpenGLWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
	PULSE_UNUSED(fences);
	PULSE_UNUSED(fences_count);
	PULSE_UNUSED(wait_for_all);
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	opengl_device->glFinish(device);
	return true;
}
