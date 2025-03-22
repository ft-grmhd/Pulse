// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_H_
#define PULSE_OPENGL_H_

#define OPENGL_RETRIEVE_DRIVER_DATA_AS(handle, cast) ((cast)handle->driver_data)

#include "EGL/EGLInstance.h"

typedef struct OpenGLDriverData
{
	union
	{
		EGLInstance egl_instance;
	};
} OpenGLDriverData;

PulseBackendFlags OpenGLCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_OPENGL in case of success and PULSE_BACKEND_INVALID otherwise
PulseBackendFlags OpenGLESCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_OPENGL_ES in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_OPENGL_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
