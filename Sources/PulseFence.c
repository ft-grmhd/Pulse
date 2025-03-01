// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseFence PulseCreateFence(PulseDevice device)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	return device->PFN_CreateFence(device);
}

PULSE_API void PulseDestroyFence(PulseDevice device, PulseFence fence)
{
	PULSE_CHECK_HANDLE(device);
	return device->PFN_DestroyFence(device, fence);
}

PULSE_API bool PulseIsFenceReady(PulseDevice device, PulseFence fence)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	PULSE_CHECK_HANDLE_RETVAL(fence, false);
	return device->PFN_IsFenceReady(device, fence);
}

PULSE_API bool PulseWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	PULSE_CHECK_PTR_RETVAL(fences, false);
	bool res = device->PFN_WaitForFences(device, fences, fences_count, wait_for_all);
	if(res)
	{
		for(uint32_t i = 0; i < fences_count; i++)
			fences[i]->cmd->state = PULSE_COMMAND_LIST_STATE_READY;
	}
	return res;
}
