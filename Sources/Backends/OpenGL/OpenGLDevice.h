// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_DEVICE_H_
#define PULSE_OPENGL_DEVICE_H_

#include <Pulse.h>
#include "OpenGL.h"
#include "EGL/EGLInstance.h"

#define PULSE_OPENGL_WRAPPER_RET(ret, fn, arg_list, param_list, cast) typedef ret (*PulseOpenGLWrapperPFN_##fn) arg_list ;
#define PULSE_OPENGL_WRAPPER(fn, arg_list, param_list, cast) typedef void (*PulseOpenGLWrapperPFN_##fn) arg_list ;
	#include "OpenGLWraps.h"
#undef PULSE_OPENGL_WRAPPER
#undef PULSE_OPENGL_WRAPPER_RET

typedef struct OpenGLDevice
{
	GLFunction original_function_ptrs[OPENGL_FUNCTION_INDEX_END_ENUM];

	union
	{
		EGLInstance egl_instance;
	};
	OpenGLContextType context_type;

	#define PULSE_OPENGL_WRAPPER_RET(ret, fn, arg_list, param_list, cast) PulseOpenGLWrapperPFN_##fn fn;
	#define PULSE_OPENGL_WRAPPER(fn, arg_list, param_list, cast) PulseOpenGLWrapperPFN_##fn fn;
		#include "OpenGLWraps.h"
	#undef PULSE_OPENGL_WRAPPER
	#undef PULSE_OPENGL_WRAPPER_RET
} OpenGLDevice;

PulseDevice OpenGLCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count);
void OpenGLDestroyDevice(PulseDevice device);

#endif // PULSE_OPENGL_DEVICE_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
