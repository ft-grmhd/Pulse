// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseImage PulseCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos)
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
	return device->PFN_CreateImage(device, create_infos);
}

PULSE_API void PulseDestroyImage(PulseDevice device, PulseImage image)
{
	PULSE_CHECK_HANDLE(device);

	if(image == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "image is NULL, this may be a bug in your application");
		return;
	}
	return device->PFN_DestroyImage(device, image);
}
