// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_EGL_CONTEXT_H_
#define PULSE_EGL_CONTEXT_H_

typedef struct EGLContext
{
	EGLDisplay display;
	EGLSurface surface;
	EGLContext handle;
} EGLContext;

#endif // PULSE_EGL_CONTEXT_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
