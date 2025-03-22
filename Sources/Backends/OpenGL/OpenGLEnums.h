// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_ENUMS_H_
#define PULSE_OPENGL_ENUMS_H_

typedef enum OpenGLContextType
{
	OPENGL_CONTEXT_EGL = 0,
	OPENGL_CONTEXT_WGL,

	OPENGL_CONTEXT_END_ENUM // For internal use only
} OpenGLContextType;

typedef enum OpenGLFunctionIndex
{
	#define PULSE_OPENGL_FUNCTION(fn, T) fn,
		#include "OpenGLFunctions.h"
	#undef PULSE_OPENGL_FUNCTION

	OPENGL_FUNCTION_INDEX_END_ENUM
} OpenGLFunctionIndex;

#endif // PULSE_OPENGL_ENUMS_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
