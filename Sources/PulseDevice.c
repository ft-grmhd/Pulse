// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "PulseInternal.h"

#define PULSE_CHECK_HANDLE_RETVAL(handle, retval) \
	do { \
		if(handle == PULSE_NULL_HANDLE) \
		{ \
			PulseSetInternalError(PULSE_ERROR_INVALID_HANDLE); \
			return retval; \
		} \
	} while(0); \

#define PULSE_CHECK_HANDLE(handle) PULSE_CHECK_HANDLE_RETVAL(handle, )

// Ordered by default preference
static const PulseBackendLoader* backends[] = {
	#ifdef PULSE_ENABLE_VULKAN_BACKEND
		&VulkanDriver,
	#endif
	#ifdef PULSE_ENABLE_D3D11_BACKEND
		&D3D11Driver,
	#endif
	PULSE_NULLPTR
};

static PulseErrorType last_error = PULSE_ERROR_NONE;

void PulseSetInternalError(PulseErrorType error)
{
	last_error = error;
}

static const PulseBackendLoader* PulseSelectBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if((backend_candidates & PULSE_BACKEND_INVALID) != 0)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return PULSE_NULLPTR;
	}
	for(int i = 0; backends[i] != PULSE_NULLPTR; i++)
	{
		if(backend_candidates != PULSE_BACKEND_ANY && (backend_candidates & backends[i]->backend) == 0)
			continue;
		if((shader_formats_used & backends[i]->supported_shader_formats) == 0)
		{
			PulseSetInternalError(PULSE_ERROR_BACKENDS_CANDIDATES_SHADER_FORMAT_MISMATCH);
			return PULSE_NULLPTR;
		}
		if(backends[i]->PFN_LoadBackend())
			return backends[i];
	}
	PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return PULSE_NULLPTR;
}

PULSE_API PulseDevice PulseCreateDevice(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used, PulseDebugLevel debug_level)
{
	const PulseBackendLoader* backend = PulseSelectBackend(backend_candidates, shader_formats_used);
	if(backend == PULSE_NULLPTR) // Error code already set by PulseSelectBackend
		return PULSE_NULL_HANDLE;

	PulseDevice device = backend->PFN_CreateDevice(debug_level);
	if(device == PULSE_NULL_HANDLE)
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return device;
}

PULSE_API void PulseDestroyDevice(PulseDevice device)
{
	PULSE_CHECK_HANDLE(device);
	device->PFN_DestroyDevice(device);
}

PULSE_API PulseBackendBits PulseGetBackendInUseByDevice(PulseDevice device)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_BACKEND_INVALID);
	return device->backend;
}

PULSE_API bool PulseSupportsBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if((backend_candidates & PULSE_BACKEND_INVALID) != 0)
		return false;
	if((backend_candidates & PULSE_BACKEND_ANY) != 0)
		return true;
	for(int i = 0; backends[i] != PULSE_NULLPTR; i++)
	{
		if((backend_candidates & backends[i]->backend) == 0)
			continue;
		if((shader_formats_used & backends[i]->supported_shader_formats) != 0)
			return true;
	}
	return false;
}

PULSE_API bool PulseDeviceSupportsSahderFormats(PulseDevice device, PulseShaderFormatsFlags shader_formats_used)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	return (device->supported_shader_formats & shader_formats_used) != 0;
}

PULSE_API PulseErrorType PulseGetLastErrorType()
{
	PulseErrorType error = last_error;
	last_error = PULSE_ERROR_NONE;
	return error;
}

PULSE_API const char* PulseVerbaliseErrorType(PulseErrorType error)
{
	switch(error)
	{
		case PULSE_ERROR_NONE:                                       return "no error";
		case PULSE_ERROR_BACKENDS_CANDIDATES_SHADER_FORMAT_MISMATCH: return "no backend candidates support the required shader formats";
		case PULSE_ERROR_INITIALIZATION_FAILED:                      return "initialization of an object could not be completed for implementation-specific reasons";

		default: return "invalid error type";
	};
	return PULSE_NULLPTR; // To avoid warnings, should be unreachable
}
