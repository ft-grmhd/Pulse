// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "PulseInternal.h"

PULSE_API PulseDevice PulseCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULL_HANDLE);
	return backend->PFN_CreateDevice(backend, forbiden_devices, forbiden_devices_count);
}

PULSE_API void PulseDestroyDevice(PulseDevice device)
{
	PULSE_CHECK_HANDLE(device);
	device->PFN_DestroyDevice(device);
	device->driver_data = PULSE_NULLPTR;
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
