// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "OpenGL.h"

PulseBackendFlags OpenGLCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && ((candidates & PULSE_BACKEND_OPENGL) == 0 || (candidates & PULSE_BACKEND_OPENGL_ES) == 0))
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_GLSL_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	EGLInstance instance;
	if(!EGLLoadInstance(&instance, candidates))
		return PULSE_BACKEND_INVALID;
	EGLUnloadInstance(&instance);
	return PULSE_BACKEND_OPENGL;
}

bool OpenGLLoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	PULSE_UNUSED(backend);
	PULSE_UNUSED(debug_level);
	OpenGLDriverData* driver_data = (OpenGLDriverData*)calloc(1, sizeof(OpenGLDriverData));
	PULSE_CHECK_ALLOCATION_RETVAL(driver_data, false);
	#ifdef PULSE_PLAT_WINDOWS
		// WGL support
	#else
		EGLLoadInstance(&driver_data->egl_instance, false);
	#endif
	OpenGLDriver.driver_data = driver_data;
	return true;
}

void OpenGLUnloadBackend(PulseBackend backend)
{
	EGLUnloadInstance(&OPENGL_RETRIEVE_DRIVER_DATA_AS(backend, OpenGLDriverData*)->egl_instance);
	free(backend->driver_data);
}

PulseBackendFlags OpenGLESCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_OPENGL) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_GLSL_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	return PULSE_BACKEND_OPENGL;
}

bool OpenGLESLoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	PULSE_UNUSED(backend);
	PULSE_UNUSED(debug_level);
	OpenGLDriverData* driver_data = (OpenGLDriverData*)calloc(1, sizeof(OpenGLDriverData));
	PULSE_CHECK_ALLOCATION_RETVAL(driver_data, false);
	EGLLoadInstance(&driver_data->egl_instance, true);
	OpenGLDriver.driver_data = driver_data;
	return true;
}

void OpenGLESUnloadBackend(PulseBackend backend)
{
	EGLUnloadInstance(&OPENGL_RETRIEVE_DRIVER_DATA_AS(backend, OpenGLDriverData*)->egl_instance);
	free(backend->driver_data);
}

PulseBackendHandler OpenGLDriver = {
	.PFN_LoadBackend = OpenGLLoadBackend,
	.PFN_UnloadBackend = OpenGLUnloadBackend,
	.PFN_CreateDevice = PULSE_NULLPTR,
	.backend = PULSE_BACKEND_OPENGL,
	.supported_shader_formats = PULSE_SHADER_FORMAT_GLSL_BIT,
	.driver_data = PULSE_NULLPTR
};

PulseBackendHandler OpenGLESDriver = {
	.PFN_LoadBackend = OpenGLESLoadBackend,
	.PFN_UnloadBackend = OpenGLESUnloadBackend,
	.PFN_CreateDevice = PULSE_NULLPTR,
	.backend = PULSE_BACKEND_OPENGL_ES,
	.supported_shader_formats = PULSE_SHADER_FORMAT_GLSL_BIT,
	.driver_data = PULSE_NULLPTR
};
