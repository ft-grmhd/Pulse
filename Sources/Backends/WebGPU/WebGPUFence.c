// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUFence.h"

PulseFence WebGPUCreateFence(PulseDevice device)
{
	PULSE_UNUSED(device);

	PulseFence fence = (PulseFence)calloc(1, sizeof(PulseFence));
	PULSE_CHECK_ALLOCATION_RETVAL(fence, PULSE_NULL_HANDLE);

	WebGPUFence* webgpu_fence = (WebGPUFence*)calloc(1, sizeof(WebGPUFence));
	PULSE_CHECK_ALLOCATION_RETVAL(webgpu_fence, PULSE_NULL_HANDLE);

	atomic_store(&webgpu_fence->signal, true);

	fence->driver_data = webgpu_fence;

	return fence;
}

void WebGPUDestroyFence(PulseDevice device, PulseFence fence)
{
	PULSE_UNUSED(device);
	free(fence->driver_data);
	free(fence);
}

bool WebGPUIsFenceReady(PulseDevice device, PulseFence fence)
{
	PULSE_UNUSED(device);
	WebGPUFence* webgpu_fence = WEBGPU_RETRIEVE_DRIVER_DATA_AS(fence, WebGPUFence*);
	return atomic_load(&webgpu_fence->signal);
}

bool WebGPUWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
	PULSE_UNUSED(device);
	if(fences_count == 0)
		return true;
	uint32_t fences_to_wait = fences_count;
	while(fences_to_wait != 0)
	{
		for(uint32_t i = 0; i < fences_count; i++)
		{
			if(WebGPUIsFenceReady(device, fences[i]))
				fences_to_wait--;
		}
		if(!wait_for_all && fences_to_wait != fences_count)
			return true;
		PulseSleep(1); // 1ms
	}
	return true;
}
