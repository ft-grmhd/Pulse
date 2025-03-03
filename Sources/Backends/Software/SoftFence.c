// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftFence.h"

PulseFence SoftCreateFence(PulseDevice device)
{
	PULSE_UNUSED(device);

	PulseFence fence = (PulseFence)calloc(1, sizeof(PulseFence));
	PULSE_CHECK_ALLOCATION_RETVAL(fence, PULSE_NULL_HANDLE);

	SoftFence* soft_fence = (SoftFence*)calloc(1, sizeof(SoftFence));
	PULSE_CHECK_ALLOCATION_RETVAL(soft_fence, PULSE_NULL_HANDLE);

	atomic_store(&soft_fence->signal, true);

	fence->driver_data = soft_fence;

	return fence;
}

void SoftDestroyFence(PulseDevice device, PulseFence fence)
{
	PULSE_UNUSED(device);
	free(fence->driver_data);
	free(fence);
}

bool SoftIsFenceReady(PulseDevice device, PulseFence fence)
{
	PULSE_UNUSED(device);
	SoftFence* soft_fence = SOFT_RETRIEVE_DRIVER_DATA_AS(fence, SoftFence*);
	return atomic_load(&soft_fence->signal);
}

bool SoftWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
	PULSE_UNUSED(device);
	if(fences_count == 0)
		return true;
	uint32_t fences_to_wait = fences_count;
	while(fences_to_wait != 0)
	{
		for(uint32_t i = 0; i < fences_count; i++)
		{
			if(SoftIsFenceReady(device, fences[i]))
				fences_to_wait--;
		}
		if(!wait_for_all && fences_to_wait != fences_count)
			return true;
		PulseSleep(1); // 1ms
	}
	return true;
}
