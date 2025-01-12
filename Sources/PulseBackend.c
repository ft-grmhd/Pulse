// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdio.h>
#include <stdarg.h>

#include <Pulse.h>
#include "PulseInternal.h"

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	#include "Backends/Vulkan/Vulkan.h"
#endif
#ifdef PULSE_ENABLE_METAL_BACKEND
	#include "Backends/Metal/Metal.h"
#endif

// Ordered by default preference
static const PulseCheckBackendSupportPFN backends_supports[] = {
	#ifdef PULSE_ENABLE_VULKAN_BACKEND
		VulkanCheckSupport,
	#endif
	#ifdef PULSE_ENABLE_METAL_BACKEND
		MetalCheckSupport,
	#endif
	PULSE_NULLPTR
};

static PulseErrorType last_error = PULSE_ERROR_NONE;

void PulseSetInternalError(PulseErrorType error)
{
	last_error = error;
}

#define LOG_MESSAGE_MAX_LENGTH 4096

void PulseLogBackend(PulseBackend backend, PulseDebugMessageSeverity type, const char* message, const char* file, const char* function, int line, ...)
{
	(void)file; // May be used later
	if(backend == PULSE_NULL_HANDLE)
		return;
	if(!backend->PFN_UserDebugCallback)
		return;

	va_list argptr;
	va_start(argptr, line);

	char complete_message[LOG_MESSAGE_MAX_LENGTH] = { 0 };

	int shift = 0;

	if(type != PULSE_DEBUG_MESSAGE_SEVERITY_INFO)
	{
		shift = snprintf(complete_message, LOG_MESSAGE_MAX_LENGTH, "[%s:%d] ", function, line);
		if(backend->debug_level == PULSE_PARANOID_DEBUG && type == PULSE_DEBUG_MESSAGE_SEVERITY_WARNING)
			type = PULSE_DEBUG_MESSAGE_SEVERITY_ERROR;
		if(shift == -1)
			shift = 0;
	}
	vsnprintf(complete_message + shift, LOG_MESSAGE_MAX_LENGTH - shift, message, argptr);
	backend->PFN_UserDebugCallback(type, complete_message);
	va_end(argptr);
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
		#ifdef PULSE_ENABLE_METAL_BACKEND
			case PULSE_BACKEND_METAL: return &MetalDriver;
		#endif

		default: break;
	}
	return PULSE_NULL_HANDLE;
}

PULSE_API PulseBackend PulseLoadBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used, PulseDebugLevel debug_level)
{
	PulseBackendFlags backend_type = PulseSelectBackend(backend_candidates, shader_formats_used);
	if(backend_type == PULSE_BACKEND_INVALID) // Error code already set by PulseSelectBackend
		return PULSE_NULL_HANDLE;

	PulseBackend backend = PulseGetBackendFromFlag(backend_type);
	if(backend == PULSE_NULL_HANDLE)
		return PULSE_NULL_HANDLE;
	if(!backend->PFN_LoadBackend(backend, debug_level))
		return PULSE_NULL_HANDLE;
	backend->PFN_UserDebugCallback = PULSE_NULLPTR;
	backend->debug_level = debug_level;
	return (PulseBackend)backend;
}

PULSE_API void PulseUnloadBackend(PulseBackend backend)
{
	PULSE_CHECK_HANDLE(backend);
	backend->PFN_UnloadBackend(backend);
}

PULSE_API PulseBackendFlags PulseGetBackendType(PulseBackend backend)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_BACKEND_INVALID);
	return backend->backend;
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

PULSE_API void PulseSetDebugCallback(PulseBackend backend, PulseDebugCallbackPFN callback)
{
	PULSE_CHECK_HANDLE(backend);
	backend->PFN_UserDebugCallback = callback;
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
		case PULSE_ERROR_INVALID_BACKEND:                            return "invalid backend";
		case PULSE_ERROR_INVALID_HANDLE:                             return "invalid handle";
		case PULSE_ERROR_BACKENDS_CANDIDATES_SHADER_FORMAT_MISMATCH: return "no backend candidates support the required shader formats";
		case PULSE_ERROR_INITIALIZATION_FAILED:                      return "initialization of an object could not be completed for implementation-specific reasons";
		case PULSE_ERROR_CPU_ALLOCATION_FAILED:                      return "an internal CPU allocation failed";
		case PULSE_ERROR_DEVICE_ALLOCATION_FAILED:                   return "a device allocation failed";
		case PULSE_ERROR_DEVICE_LOST:                                return "device has been lost";
		case PULSE_ERROR_INVALID_INTERNAL_POINTER:                   return "invalid internal pointer";
		case PULSE_ERROR_MAP_FAILED:                                 return "memory mapping failed";
		case PULSE_ERROR_INVALID_DEVICE:                             return "device is invalid";

		default: return "invalid error type";
	};
	return PULSE_NULLPTR; // To avoid warnings, should be unreachable
}
