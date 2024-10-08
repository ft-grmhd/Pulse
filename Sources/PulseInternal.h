// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_INTERNAL_H_
#define PULSE_INTERNAL_H_

#include <Pulse.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*PulseLoadBackendPFN)(void);
typedef PulseDevice (*PulseCreateDevicePFN)(PulseDebugLevel);

typedef void (*PulseDestroyDevicePFN)(PulseDevice);

typedef struct PulseDeviceHandler
{
	// PFNs
	PulseDestroyDevicePFN PFN_DestroyDevice;

	// Attributes
	PulseBackendFlags backend;
	PulseShaderFormatsFlags supported_shader_formats;
	PulseDebugLevel debug_level;
} PulseDeviceHandler;

typedef struct PulseBackendLoader
{
	// PFNs
	PulseLoadBackendPFN PFN_LoadBackend;
	PulseCreateDevicePFN PFN_CreateDevice;

	// Attributes
	PulseBackendFlags backend;
	PulseShaderFormatsFlags supported_shader_formats;
} PulseBackendLoader;

void PulseSetInternalError(PulseErrorType error);

#define PULSE_LOAD_DRIVER_DEVICE_FUNCTION(fn, _namespace) device->PFN_##fn = _namespace##fn;
#define PULSE_LOAD_DRIVER_DEVICE(_namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyDevice, _namespace) \

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	extern PulseBackendLoader VulkanDriver;
#endif // PULSE_ENABLE_VULKAN_BACKEND
#ifdef PULSE_ENABLE_D3D11_BACKEND
	extern PulseBackendLoader D3D11Driver;
#endif // PULSE_ENABLE_D3D11_BACKEND

#ifdef __cplusplus
}
#endif

#endif // PULSE_INTERNAL_H_
