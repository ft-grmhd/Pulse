// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_EGL_CONTEXT_H_
#define PULSE_EGL_CONTEXT_H_

#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#include <Pulse.h>

typedef struct EGLInstance
{
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig config;

	#define PULSE_EGL_FUNCTION(fn, T) T fn;
		#include "EGLFunctions.h"
	#undef PULSE_EGL_FUNCTION
} EGLInstance;

bool EGLLoadInstance(EGLInstance* instance, bool es_context);
void EGLUnloadInstance(EGLInstance* instance);

#endif // PULSE_EGL_CONTEXT_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
