// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_EGL_CONTEXT_H_
#define PULSE_EGL_CONTEXT_H_

#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <Pulse.h>

typedef struct EGLInstance
{
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig config;

	EGLDeviceEXT device;

	#define PULSE_EGL_FUNCTION(fn, T) T fn;
	#define PULSE_EGL_FUNCTION_EXT(fn, T) T fn;
		#include "EGLFunctions.h"
	#undef PULSE_EGL_FUNCTION
	#undef PULSE_EGL_FUNCTION_EXT
} EGLInstance;

bool EGLLoadInstance(EGLInstance* instance, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count, bool es_context);
void EGLUnloadInstance(EGLInstance* instance);

#endif // PULSE_EGL_CONTEXT_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
