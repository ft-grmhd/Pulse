// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "WebGPU.h"
#include "WebGPUDevice.h"

PulseBackendFlags WebGPUCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_WEBGPU) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_WGSL_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	WGPUInstance instance = wgpuCreateInstance(PULSE_NULLPTR);
	if(instance == PULSE_NULLPTR)
		return PULSE_BACKEND_INVALID;
	wgpuInstanceRelease(instance);
	return PULSE_BACKEND_WEBGPU;
}

bool WebGPULoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	WebGPUDriverData* driver_data = (WebGPUDriverData*)calloc(1, sizeof(WebGPUDriverData));
	PULSE_CHECK_ALLOCATION_RETVAL(driver_data, false);
	driver_data->instance = wgpuCreateInstance(PULSE_NULLPTR);
	if(driver_data->instance == PULSE_NULLPTR)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		free(driver_data);
		return false;
	}
	WebGPUDriver.driver_data = driver_data;
	return true;
}

void WebGPUUnloadBackend(PulseBackend backend)
{
	wgpuInstanceRelease(WEBGPU_RETRIEVE_DRIVER_DATA_AS(backend, WebGPUDriverData*)->instance);
	free(backend->driver_data);
}

PulseBackendHandler WebGPUDriver = {
	.PFN_LoadBackend = WebGPULoadBackend,
	.PFN_UnloadBackend = WebGPUUnloadBackend,
	.PFN_CreateDevice = WebGPUCreateDevice,
	.backend = PULSE_BACKEND_WEBGPU,
	.supported_shader_formats = PULSE_SHADER_FORMAT_WGSL_BIT,
	.driver_data = PULSE_NULLPTR
};
