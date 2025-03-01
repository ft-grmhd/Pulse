// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_H_
#define PULSE_WEBGPU_H_

#include <webgpu/webgpu.h>

#define WEBGPU_RETRIEVE_DRIVER_DATA_AS(handle, cast) ((cast)handle->driver_data)

typedef struct WebGPUDriverData
{
	WGPUInstance instance;
} WebGPUDriverData;

PulseBackendFlags WebGPUCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_WEBGPU in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_WEBGPU_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
