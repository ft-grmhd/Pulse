// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "OpenGL.h"
#include "OpenGLDevice.h"

PulseBackendFlags OpenGLCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_OPENGL) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_GLSL_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	PulseDevice device = OpenGLCreateDevice(&OpenGLDriver, PULSE_NULLPTR, 0);
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_BACKEND_INVALID);
	OpenGLDestroyDevice(device);

	return PULSE_BACKEND_OPENGL;
}

PulseBackendFlags OpenGLESCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_OPENGL_ES) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_GLSL_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	PulseDevice device = OpenGLCreateDevice(&OpenGLESDriver, PULSE_NULLPTR, 0);
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_BACKEND_INVALID);
	OpenGLDestroyDevice(device);

	return PULSE_BACKEND_OPENGL_ES;
}

bool OpenGLLoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	PULSE_UNUSED(backend);
	PULSE_UNUSED(debug_level);
	return true;
}

void OpenGLUnloadBackend(PulseBackend backend)
{
	PULSE_UNUSED(backend);
}

PulseBackendHandler OpenGLDriver = {
	.PFN_LoadBackend = OpenGLLoadBackend,
	.PFN_UnloadBackend = OpenGLUnloadBackend,
	.PFN_CreateDevice = OpenGLCreateDevice,
	.backend = PULSE_BACKEND_OPENGL,
	.supported_shader_formats = PULSE_SHADER_FORMAT_GLSL_BIT,
	.driver_data = PULSE_NULLPTR
};

PulseBackendHandler OpenGLESDriver = {
	.PFN_LoadBackend = OpenGLLoadBackend,
	.PFN_UnloadBackend = OpenGLUnloadBackend,
	.PFN_CreateDevice = OpenGLCreateDevice,
	.backend = PULSE_BACKEND_OPENGL_ES,
	.supported_shader_formats = PULSE_SHADER_FORMAT_GLSL_BIT,
	.driver_data = PULSE_NULLPTR
};
