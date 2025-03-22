// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "EGLInstance.h"
#include "../../../PulseInternal.h"

static PulseLibModule egl_lib_module = PULSE_NULL_LIB_MODULE;
static uint32_t loader_references_count = 0;

static bool EGLLoadFunctions(EGLInstance* instance)
{
	#ifdef PULSE_PLAT_WINDOWS
		const char* libnames[] = {
			"libEGL.dll"
		};
	#else
		const char* libnames[] = {
			"libEGL.so.1",
			"libEGL.so"
		};
	#endif


	for(size_t i = 0; i < sizeof(libnames) / sizeof(const char*); i++)
	{
		if(loader_references_count == 0)
			egl_lib_module = PulseLoadLibrary(libnames[i]);
		if(egl_lib_module)
		{
			instance->eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC)PulseLoadSymbolFromLibModule(egl_lib_module, "eglGetProcAddress");
			if(instance->eglGetProcAddress == PULSE_NULLPTR)
			{
				PulseUnloadLibrary(egl_lib_module);
				egl_lib_module = PULSE_NULL_LIB_MODULE;
			}
			else
				break;
		}
	}
	if(!egl_lib_module)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return false;
	}
	loader_references_count++;

	#define PULSE_EGL_FUNCTION(fn, T) \
		instance->fn = (T)instance->eglGetProcAddress(#fn); \
		if(!instance->fn) \
			return false;
		#include "EGLFunctions.h"
	#undef PULSE_EGL_FUNCTION

	return true;
}

bool EGLLoadInstance(EGLInstance* instance, bool es_context)
{
	PULSE_CHECK_PTR_RETVAL(instance, false);

	if(!EGLLoadFunctions(instance))
		return false;

	instance->display = instance->eglGetDisplay(EGL_DEFAULT_DISPLAY);
	PULSE_CHECK_PTR_RETVAL(instance->display, false);

	instance->eglInitialize(instance->display, PULSE_NULLPTR, PULSE_NULLPTR);
	EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, es_context ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT,
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_NONE
	};
	EGLint num_configs;
	instance->eglChooseConfig(instance->display, attribs, &instance->config, 1, &num_configs);
	EGLint ctxAttribs[] = {
		es_context ? EGL_CONTEXT_CLIENT_VERSION : EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_NONE
	};
	instance->eglBindAPI(es_context ? EGL_OPENGL_ES_API : EGL_OPENGL_API);
	instance->context = instance->eglCreateContext(instance->display, instance->config, EGL_NO_CONTEXT, ctxAttribs);
	PULSE_CHECK_PTR_RETVAL(instance->context, false);

	EGLint pbufferAttribs[] = {
		EGL_WIDTH, 1,
		EGL_HEIGHT, 1,
		EGL_NONE
	};
	instance->surface = instance->eglCreatePbufferSurface(instance->display, instance->config, pbufferAttribs);
	PULSE_CHECK_PTR_RETVAL(instance->surface, false);

	return instance->eglMakeCurrent(instance->display, instance->surface, instance->surface, instance->context);
}

void EGLUnloadInstance(EGLInstance* instance)
{
	PULSE_CHECK_PTR(instance);
	instance->eglDestroySurface(instance->display, instance->surface);
	instance->eglDestroyContext(instance->display, instance->context);
	instance->eglTerminate(instance->display);

	loader_references_count--;
	if(loader_references_count != 0)
		return;
	PulseUnloadLibrary(egl_lib_module);
	egl_lib_module = PULSE_NULL_LIB_MODULE;
}
