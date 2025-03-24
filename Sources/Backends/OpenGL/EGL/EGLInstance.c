// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdio.h>
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

bool EGLLoadOpenGLContext(EGLInstance* instance, EGLDisplay* display, EGLDeviceEXT device, EGLConfig* config, EGLSurface* surface, EGLContext* context, bool es_context)
{
	if(device != EGL_NO_DEVICE_EXT)
		*display = instance->eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, device, PULSE_NULLPTR);
	else
		*display = instance->eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(display == EGL_NO_DISPLAY || !instance->eglInitialize(*display, PULSE_NULLPTR, PULSE_NULLPTR))
		return false;

	EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, es_context ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT,
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_NONE
	};
	EGLint num_configs;
	instance->eglChooseConfig(*display, attribs, config, 1, &num_configs);
	instance->eglBindAPI(es_context ? EGL_OPENGL_ES_API : EGL_OPENGL_API);
	EGLint pbufferAttribs[] = {
		EGL_WIDTH, 1,
		EGL_HEIGHT, 1,
		EGL_NONE
	};
	*surface = instance->eglCreatePbufferSurface(*display, *config, pbufferAttribs);
	if(es_context)
	{
		EGLint ctx_attribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3,
			EGL_NONE
		};
		*context = instance->eglCreateContext(*display, *config, EGL_NO_CONTEXT, ctx_attribs);
	}
	else
	{
		EGLint ctx_attribs[] = {
			EGL_CONTEXT_MAJOR_VERSION, 4,
			EGL_CONTEXT_MINOR_VERSION_KHR, 3,
			EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
			EGL_NONE
		};
		*context = instance->eglCreateContext(*display, *config, EGL_NO_CONTEXT, ctx_attribs);
	}
	if(*context == EGL_NO_CONTEXT)
	{
		instance->eglDestroySurface(*display, *surface);
		instance->eglTerminate(*display);
		return false;
	}
	if(!instance->eglMakeCurrent(*display, *surface, *surface, *context))
	{
		instance->eglDestroySurface(*display, *surface);
		instance->eglDestroyContext(display, *context);
		instance->eglTerminate(*display);
		return false;
	}
	return true;
}

bool EGLLoadInstance(EGLInstance* instance, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count, bool es_context)
{
	PULSE_CHECK_PTR_RETVAL(instance, false);

	if(!EGLLoadFunctions(instance))
		return false;

	instance->device = EGL_NO_DEVICE_EXT;

	if(instance->eglGetPlatformDisplayEXT && instance->eglQueryDevicesEXT)
	{
		EGLDeviceEXT* devices = PULSE_NULLPTR;
		int32_t device_count;
		uint64_t best_device_score = 0;

		instance->eglQueryDevicesEXT(0, PULSE_NULLPTR, &device_count);
		devices = (EGLDeviceEXT*)calloc(device_count, sizeof(EGLDeviceEXT));
		PULSE_CHECK_ALLOCATION_RETVAL(devices, false);
		instance->eglQueryDevicesEXT(device_count, devices, &device_count);

		for(int32_t i = 0; i < device_count; i++)
		{
			EGLDisplay display;
			EGLConfig config;
			EGLSurface surface;
			EGLContext context;
			if(!EGLLoadOpenGLContext(instance, &display, devices[i], &config, &surface, &context, es_context))
				continue;

			PFNGLGETINTEGERI_VPROC glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)instance->eglGetProcAddress("glGetIntegeri_v");
			PFNGLGETINTEGERVPROC glGetIntegerv = (PFNGLGETINTEGERVPROC)instance->eglGetProcAddress("glGetIntegerv");
			PFNGLGETSTRINGPROC glGetString = (PFNGLGETSTRINGPROC)instance->eglGetProcAddress("glGetString");
			PFNGLGETSTRINGIPROC glGetStringi = (PFNGLGETSTRINGIPROC)instance->eglGetProcAddress("glGetStringi");

			if(!glGetIntegeri_v || !glGetIntegerv || !glGetString || !glGetStringi)
			{
				instance->eglDestroySurface(display, surface);
				instance->eglDestroyContext(display, context);
				instance->eglTerminate(display);
				continue;
			}

			// Check for forbiden devices
			{
				uint32_t device_id = PulseHashString((const char*)glGetString(GL_VENDOR));
				device_id = PulseHashCombine(device_id, PulseHashString((const char*)glGetString(GL_RENDERER)));
				GLint gl_extension_count = 0;
				glGetIntegerv(GL_NUM_EXTENSIONS, &gl_extension_count);
				for(int i = 0; i < gl_extension_count; i++)
					device_id = PulseHashCombine(device_id, PulseHashString((const char*)glGetStringi(GL_EXTENSIONS, i)));

				for(uint32_t j = 0; j < forbiden_devices_count; j++)
				{
					OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(forbiden_devices[j], OpenGLDevice*);
					if(opengl_device->context_type != OPENGL_CONTEXT_EGL)
						continue;
					if(device_id == opengl_device->device_id)
						return true;
				}
			}

			uint64_t current_device_score = 0;

			const char* vendor = (const char*)glGetString(GL_VENDOR);
			if(vendor && (strstr(vendor, "NVIDIA") || strstr(vendor, "AMD") || strstr(vendor, "ATI"))) // High chances of being a discrete GPU
				current_device_score += 10000;
			else if(vendor && strstr(vendor, "Intel")) // High chances of being an integrated GPU
				current_device_score += 1000;

			GLint max_compute_work_group_invocations;
			glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

			GLint max_compute_work_group_size_x;
			GLint max_compute_work_group_size_y;
			GLint max_compute_work_group_size_z;
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &max_compute_work_group_size_x);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &max_compute_work_group_size_y);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &max_compute_work_group_size_z);

			GLint max_texture_size;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

			current_device_score += max_compute_work_group_invocations;
			current_device_score += max_compute_work_group_size_x;
			current_device_score += max_compute_work_group_size_y;
			current_device_score += max_compute_work_group_size_z;
			current_device_score += max_texture_size;

			instance->eglDestroySurface(display, surface);
			instance->eglDestroyContext(display, context);
			instance->eglTerminate(display);

			if(current_device_score > best_device_score)
			{
				best_device_score = current_device_score;
				instance->device = devices[i];
			}
		}
	}

	return EGLLoadOpenGLContext(instance, &instance->display, instance->device, &instance->config, &instance->surface, &instance->context, es_context);
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
