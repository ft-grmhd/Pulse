// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseDevice PulseCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULL_HANDLE);
	return backend->PFN_CreateDevice(backend, forbiden_devices, forbiden_devices_count);
}

PULSE_API void PulseDestroyDevice(PulseDevice device)
{
	PULSE_CHECK_HANDLE(device);
	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(device->allocated_buffers_size != 0)
			PulseLogErrorFmt(device->backend, "some buffers allocated using device [%p] were not freed (%d active allocations)", device, device->allocated_buffers_size);
		if(device->allocated_images_size != 0)
			PulseLogErrorFmt(device->backend, "some images allocated using device [%p] were not freed (%d active allocations)", device, device->allocated_images_size);
	}
	free(device->allocated_buffers);
	free(device->allocated_images);
	device->PFN_DestroyDevice(device);
}

PULSE_API PulseBackendBits PulseGetBackendInUseByDevice(PulseDevice device)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_BACKEND_INVALID);
	return device->backend->backend;
}

PULSE_API bool PulseDeviceSupportsShaderFormats(PulseDevice device, PulseShaderFormatsFlags shader_formats_used)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	return (device->backend->supported_shader_formats & shader_formats_used) != 0;
}
