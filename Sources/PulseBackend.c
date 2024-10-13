// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

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

struct
{
	char file[1024];
	char function[1024];
	PulseErrorType type;
	int line;
} last_error = { .file = { 0 }, .function = { 0 }, .type = PULSE_ERROR_NONE, .line = -1 };

void PulseSetInternalErrorBackend(PulseErrorType error, const char* file, const char* function, int line)
{
	strcpy(last_error.file, file);
	strcpy(last_error.function, function);
	last_error.type = error;
	last_error.line = line;
}

void PulseLogErrorBackend(PulseBackend backend, PulseErrorType error, const char* file, const char* function, int line)
{
	if(backend == PULSE_NULL_HANDLE)
		return;
	if(!backend->PFN_UserDebugCallback)
		return;
}

void PulseLogWarningBackend(PulseBackend backend, PulseWarningType warning, const char* file, const char* function, int line)
{
}

void PulseLogInfoBackend(PulseBackend backend, const char* message, const char* file, const char* function, int line)
{
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

static const char* PulseVerbaliseErrorType(PulseErrorType error)
{
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
	if(!backend->PFN_LoadBackend(debug_level))
		return PULSE_NULL_HANDLE;
	backend->PFN_UserDebugCallback = PULSE_NULLPTR;
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

PULSE_API void PulseSetDebugCallback(PulseBackend backend, PulseDebugCallbackPFN callback)
{
	PULSE_CHECK_HANDLE(backend);
	backend->PFN_UserDebugCallback = callback;
}
