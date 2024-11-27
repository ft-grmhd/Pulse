// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "PulseInternal.h"

PULSE_API PulseBuffer PulseCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(create_infos == PULSE_NULLPTR)
		{
			PulseLogError(device->backend, "create_infos is NULL");
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			return PULSE_NULL_HANDLE;
		}
	}
	return device->PFN_CreateBuffer(device, create_infos);
}

PULSE_API void PulseDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	PULSE_CHECK_HANDLE(device);

	if(buffer == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "buffer is NULL, this may be a bug in your application");
		return;
	}
	return device->PFN_DestroyBuffer(device, buffer);
}
