// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_INTERNAL_H_
#define PULSE_INTERNAL_H_

#include <Pulse.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PULSE_CHECK_ALLOCATION_RETVAL(ptr, retval) \
	do { \
		if(ptr == PULSE_NULLPTR) \
		{ \
			PulseSetInternalError(PULSE_ERROR_ALLOCATION_FAILED); \
			return retval; \
		} \
	} while(0); \

#define PULSE_CHECK_ALLOCATION(ptr) PULSE_CHECK_ALLOCATION_RETVAL(ptr, )

typedef bool (*PulseLoadBackendPFN)(void);
typedef PulseDevice (*PulseCreateDevicePFN)(PulseDebugLevel);
typedef void (*PulseDestroyDevicePFN)(PulseDevice);

typedef struct PulseBackendLoader
{
	// PFNs
	PulseLoadBackendPFN PFN_LoadBackend;
	PulseCreateDevicePFN PFN_CreateDevice;
	PulseDestroyDevicePFN PFN_DestroyDevice;

	// Attributes
	PulseBackendFlags backend;
	PulseShaderFormatsFlags supported_shader_formats;
} PulseBackendLoader;

typedef struct PulseDeviceHandler
{
	// PFNs

	// Attributes
	void* driver_data;
	const PulseBackendLoader* backend;
	PulseDebugLevel debug_level;
} PulseDeviceHandler;

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
