// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include "EGLInstance.h"
#include "../../../PulseInternal.h"
#include "EGL/eglext.h"
#include "../OpenGLDevice.h"

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
	#define PULSE_EGL_FUNCTION_EXT(fn, T) instance->fn = (T)instance->eglGetProcAddress(#fn);
		#include "EGLFunctions.h"
	#undef PULSE_EGL_FUNCTION
	#undef PULSE_EGL_FUNCTION_EXT

	return true;
}

static bool EGLIsDeviceForbidden(EGLInstance* instance, EGLDeviceEXT device, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	if(device == EGL_NO_DEVICE_EXT)
		return true;

	const char* test_device_vendor = instance->eglQueryDeviceStringEXT(device, EGL_VENDOR);

	for(uint32_t i = 0; i < forbiden_devices_count; i++)
	{
		OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(forbiden_devices[i], OpenGLDevice*);
		if(opengl_device->context_type != OPENGL_CONTEXT_EGL)
			continue;

		const char* device_vendor = instance->eglQueryDeviceStringEXT(opengl_device->egl_instance.device, EGL_VENDOR);
		if(device_vendor && test_device_vendor && strcmp(test_device_vendor, device_vendor) == 0)
			return true;
	}
	return false;
}

#include <stdio.h>

bool EGLLoadInstance(EGLInstance* instance, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count, bool es_context)
{
	PULSE_CHECK_PTR_RETVAL(instance, false);

	if(!EGLLoadFunctions(instance))
		return false;

	instance->device = EGL_NO_DEVICE_EXT;

	if(instance->eglGetPlatformDisplayEXT && instance->eglQueryDevicesEXT && instance->eglQueryDeviceStringEXT)
	{
		EGLDeviceEXT* devices = PULSE_NULLPTR;
		EGLDeviceEXT chosen_one = EGL_NO_DEVICE_EXT;
		int32_t device_count;
		uint64_t best_device_score = 0;

		instance->eglQueryDevicesEXT(0, PULSE_NULLPTR, &device_count);
		devices = (EGLDeviceEXT*)calloc(device_count, sizeof(EGLDeviceEXT));
		PULSE_CHECK_ALLOCATION_RETVAL(devices, false);
		instance->eglQueryDevicesEXT(device_count, devices, &device_count);

		for(int32_t i = 0; i < device_count; i++)
		{
			if(EGLIsDeviceForbidden(instance, devices[i], forbiden_devices, forbiden_devices_count))
				continue;
			const char* exts = instance->eglQueryDeviceStringEXT(devices[i], EGL_EXTENSIONS);
			uint64_t current_device_score = 0;
			if(strstr(exts, "EGL_EXT_device_drm")) // tricky way to check if it is a discrete GPU
				current_device_score += 10000;
			if(current_device_score > best_device_score)
			{
				best_device_score = current_device_score;
				chosen_one = devices[i];
			}
		}

		instance->display = instance->eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, chosen_one, PULSE_NULLPTR);
	}
	else
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
