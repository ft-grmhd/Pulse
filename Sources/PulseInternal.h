// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_INTERNAL_H_
#define PULSE_INTERNAL_H_

#include <Pulse.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PulseStaticAllocStack(size) ((char[size]){ 0 })

#define PULSE_CHECK_ALLOCATION_RETVAL(ptr, retval) \
	do { \
		if(ptr == PULSE_NULLPTR) \
		{ \
			PulseSetInternalError(PULSE_ERROR_ALLOCATION_FAILED); \
			return retval; \
		} \
	} while(0); \

#define PULSE_CHECK_ALLOCATION(ptr) PULSE_CHECK_ALLOCATION_RETVAL(ptr, )

#define PULSE_CHECK_HANDLE_RETVAL(handle, retval) \
	do { \
		if(handle == PULSE_NULL_HANDLE) \
		{ \
			PulseSetInternalError(PULSE_ERROR_INVALID_HANDLE); \
			return retval; \
		} \
	} while(0); \

#define PULSE_CHECK_HANDLE(handle) PULSE_CHECK_HANDLE_RETVAL(handle, )

typedef PulseBackendFlags (*PulseCheckBackendSupportPFN)(PulseBackendFlags, PulseShaderFormatsFlags);

typedef bool (*PulseLoadBackendPFN)(PulseDebugLevel);
typedef void (*PulseUnloadBackendPFN)(PulseBackend);
typedef PulseDevice (*PulseCreateDevicePFN)(PulseBackend, PulseDevice*, uint32_t);

typedef void (*PulseDestroyDevicePFN)(PulseDevice);
typedef PulseComputePipeline (*PulseCreateComputePipelinePFN)(PulseDevice, const PulseComputePipelineCreateInfo*);
typedef void (*PulseBindComputePipelinePFN)(PulseComputePass, PulseComputePipeline);
typedef void (*PulseDestroyComputePipelinePFN)(PulseDevice, PulseComputePipeline);

typedef struct PulseBackendHandler
{
	// PFNs
	PulseLoadBackendPFN PFN_LoadBackend;
	PulseUnloadBackendPFN PFN_UnloadBackend;
	PulseCreateDevicePFN PFN_CreateDevice;

	// Attributes
	PulseBackendFlags backend;
	PulseShaderFormatsFlags supported_shader_formats;
	void* driver_data;
} PulseBackendHandler;

typedef struct PulseDeviceHandler
{
	// PFNs
	PulseDestroyDevicePFN PFN_DestroyDevice;
	PulseCreateComputePipelinePFN PFN_CreateComputePipeline;
	PulseBindComputePipelinePFN PFN_BindComputePipeline;
	PulseDestroyComputePipelinePFN PFN_DestroyComputePipeline;

	// Attributes
	void* driver_data;
	PulseBackend backend;
} PulseDeviceHandler;

void PulseSetInternalError(PulseErrorType error);

#define PULSE_LOAD_DRIVER_DEVICE_FUNCTION(fn, _namespace) pulse_device->PFN_##fn = _namespace##fn;
#define PULSE_LOAD_DRIVER_DEVICE(_namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyDevice, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(BindComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyComputePipeline, _namespace) \

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	extern PulseBackendHandler VulkanDriver;
#endif // PULSE_ENABLE_VULKAN_BACKEND
#ifdef PULSE_ENABLE_D3D11_BACKEND
	extern PulseBackendHandler D3D11Driver;
#endif // PULSE_ENABLE_D3D11_BACKEND

#ifdef __cplusplus
}
#endif

#endif // PULSE_INTERNAL_H_
