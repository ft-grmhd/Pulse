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
void WebGPUDeviceTick(PulseDevice device);

uint32_t WebGPUBytesPerRow(int32_t width, PulseImageFormat format);
uint32_t WebGPUImageFormatTexelBlockSize(PulseImageFormat format);
int32_t WebGPUGetImageBlockHeight(PulseImageFormat format);
int32_t WebGPUGetImageBlockWidth(PulseImageFormat format);

#endif // PULSE_WEBGPU_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
