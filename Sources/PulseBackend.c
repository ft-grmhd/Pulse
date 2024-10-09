// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "PulseInternal.h"

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	#include "Backends/Vulkan/Vulkan.h"
#endif
#ifdef PULSE_ENABLE_D3D11_BACKEND
	#include "Backends/D3D11/D3D11.h"
#endif

// Ordered by default preference
static const PulseCheckBackendSupportPFN backends_supports[] = {
	#ifdef PULSE_ENABLE_VULKAN_BACKEND
		VulkanCheckSupport,
	#endif
	#ifdef PULSE_ENABLE_D3D11_BACKEND
		D3D11CheckSupport,
	#endif
	PULSE_NULLPTR
};

static PulseErrorType last_error = PULSE_ERROR_NONE;

void PulseSetInternalError(PulseErrorType error)
{
	last_error = error;
}

static PulseBackendFlags PulseSelectBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if((backend_candidates & PULSE_BACKEND_INVALID) != 0)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return PULSE_BACKEND_INVALID;
	}
	for(int i = 0; backends_supports[i] != PULSE_NULLPTR; i++)
	{
		PulseBackendFlags backend = backends_supports[i](backend_candidates, shader_formats_used);
		if(backend != PULSE_BACKEND_INVALID)
			return backend;
	}
	PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return PULSE_BACKEND_INVALID;
}

static PulseBackend PulseGetBackendFromFlag(PulseBackendBits flag)
{
	switch(flag)
	{
		#ifdef PULSE_ENABLE_VULKAN_BACKEND
			case PULSE_BACKEND_VULKAN: return &VulkanDriver;
		#endif
		#ifdef PULSE_ENABLE_D3D11_BACKEND
			case PULSE_BACKEND_VULKAN: return &D3D11Driver;
		#endif

		default: return PULSE_NULL_HANDLE;
	}
	return PULSE_NULL_HANDLE; // To avoid warnings
}

PULSE_API PulseBackend PulseLoadBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used, PulseDebugLevel debug_level)
{
	PulseBackendFlags backend_type = PulseSelectBackend(backend_candidates, shader_formats_used);
	if(backend_type == PULSE_BACKEND_INVALID) // Error code already set by PulseSelectBackend
		return PULSE_NULL_HANDLE;

	PulseBackend backend = PulseGetBackendFromFlag(backend_type);
	if(backend == PULSE_NULL_HANDLE)
		return PULSE_NULL_HANDLE;
	if(!backend->PFN_LoadBackend(debug_level))
		return PULSE_NULL_HANDLE;
	return (PulseBackend)backend;
}

PULSE_API void PulseUnloadBackend(PulseBackend backend)
{
	PULSE_CHECK_HANDLE(backend);
	backend->PFN_UnloadBackend(backend);
}

PULSE_API bool PulseSupportsBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if((backend_candidates & PULSE_BACKEND_INVALID) != 0)
		return false;
	for(int i = 0; backends_supports[i] != PULSE_NULLPTR; i++)
	{
		PulseBackendFlags backend = backends_supports[i](backend_candidates, shader_formats_used);
		if(backend != PULSE_BACKEND_INVALID)
			return true;
	}
	return false;
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
		case PULSE_ERROR_ALLOCATION_FAILED:                          return "an internal allocation failed";

		default: return "invalid error type";
	};
	return PULSE_NULLPTR; // To avoid warnings, should be unreachable
}
