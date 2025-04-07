// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdio.h>
#include <string.h>

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLComputePipeline.h"
#include "OpenGLCommandList.h"
#include "OpenGLDevice.h"
#include "OpenGLFence.h"
#include "OpenGLBuffer.h"
#include "OpenGLImage.h"
#include "OpenGLComputePass.h"

static const char* OpenGLFunctionIndexToFunctionName[] = {
	#define PULSE_OPENGL_FUNCTION(fn, T) #fn,
		#include "OpenGLFunctions.h"
	#undef PULSE_OPENGL_FUNCTION
};

typedef GLFunction(*GLFunctionLoad)(const char*);

const char* OpenGLVerbaliseError(GLenum code)
{
	switch (code)
	{
		// OpenGL / OpenGL ES error codes
		case GL_INVALID_ENUM:      return "(GL_INVALID_ENUM)      an unacceptable value is specified for an enumerated argument";
		case GL_INVALID_VALUE:     return "(GL_INVALID_VALUE)     a numeric argument is out of range";
		case GL_INVALID_OPERATION: return "(GL_INVALID_OPERATION) the specified operation is not allowed in the current state";
		case GL_OUT_OF_MEMORY:     return "(GL_OUT_OF_MEMORY)     there is not enough memory left to execute the command";

		// OpenGL error codes
		case GL_STACK_UNDERFLOW:   return "(GL_STACK_UNDERFLOW)   an attempt has been made to perform an operation that would cause an internal stack to underflow";
		case GL_STACK_OVERFLOW:    return "(GL_STACK_OVERFLOW)    an attempt has been made to perform an operation that would cause an internal stack to overflow";

		default: break;
	}

	return "unknown OpenGL error";
}

static void OpenGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	PULSE_UNUSED(source);
	PULSE_UNUSED(type);
	PULSE_UNUSED(id);
	PULSE_UNUSED(severity);
	PulseDevice device = (PulseDevice)user_param;
	PulseLogInfoFmt(device->backend, "%s debug message catched: %.*s", device->backend->backend == PULSE_BACKEND_OPENGL ? "(OpenGL)" : "(OpenGL ES)", length, message);
}

static void PulseCheckGLError(PulseDevice device, const char* function)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	GLenum err = ((PFNGLGETERRORPROC)opengl_device->original_function_ptrs[glGetError])();
	if(err == GL_NO_ERROR)
		return;

	char message[4096] = { 0 };
	snprintf(message, 4096, "%s call to %s failed. Unrolling the error stack:\n", device->backend->backend == PULSE_BACKEND_OPENGL ? "(OpenGL)" : "(OpenGL ES)", function);

	int i = 0;
	for(; err != GL_NO_ERROR; err = ((PFNGLGETERRORPROC)opengl_device->original_function_ptrs[glGetError])(), i++)
		snprintf(message + strlen(message), 4096 - strlen(message), "    #%d %s\n", i, OpenGLVerbaliseError(err));

	PulseLogBackend(device->backend, PULSE_DEBUG_MESSAGE_SEVERITY_ERROR, message, __FILE__, function, 0);
}

static void OpenGLDeviceMakeCurrent(PulseDevice device)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	#ifdef PULSE_PLAT_WINDOWS
		if(opengl_device->context_type == OPENGL_CONTEXT_WGL)
		{} // TODO: WGL
		else
		{
			EGLInstance* instance = &opengl_device->egl_instance;
			instance->eglMakeCurrent(instance->display, instance->surface, instance->surface, instance->context);
		}
	#else
		EGLInstance* instance = &opengl_device->egl_instance;
		instance->eglMakeCurrent(instance->display, instance->surface, instance->surface, instance->context);
	#endif
}

#define PULSE_OPENGL_WRAPPER_RET(ret, fn, arg_list, param_list, cast) \
	static ret PulseOpenGLWrapper_##fn arg_list \
	{ \
		OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*); \
		OpenGLDeviceMakeCurrent(device); \
		ret result = ((cast)opengl_device->original_function_ptrs[fn]) param_list; \
		if(device->backend->debug_level != PULSE_NO_DEBUG) \
			PulseCheckGLError(device, #fn); \
		return result; \
	}

#define PULSE_OPENGL_WRAPPER(fn, arg_list, param_list, cast) \
	static void PulseOpenGLWrapper_##fn arg_list \
	{ \
		OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*); \
		OpenGLDeviceMakeCurrent(device); \
		((cast)opengl_device->original_function_ptrs[fn]) param_list; \
		if(device->backend->debug_level != PULSE_NO_DEBUG) \
			PulseCheckGLError(device, #fn); \
	}

#include "OpenGLWraps.h"

#undef PULSE_OPENGL_WRAPPER
#undef PULSE_OPENGL_WRAPPER_RET

static bool OpenGLLoadFallbackFunction(PulseDevice device, OpenGLFunctionIndex index, GLFunctionLoad load)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	if(index == glDebugMessageCallback)
	{
		GLFunction fn = PULSE_NULLPTR;

		if(OpenGLDeviceSupportsExtension(device, "GL_KHR_debug"))
			fn = load("glDebugMessageCallbackKHR");
		if(!fn && device->backend->backend == PULSE_BACKEND_OPENGL && OpenGLDeviceSupportsExtension(device, "GL_ARB_debug_output"))
			fn = load("glDebugMessageCallbackARB");

		if(fn)
			opengl_device->original_function_ptrs[index] = fn;
	}

	return opengl_device->original_function_ptrs[index] != PULSE_NULLPTR;
}

static bool OpenGLLoadFunction(PulseDevice device, OpenGLFunctionIndex index)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	GLFunctionLoad load = PULSE_NULLPTR;

	#ifdef PULSE_PLAT_WINDOWS
		if(opengl_device->context_type == OPENGL_CONTEXT_WGL)
		{} // TODO: WGL
		else
			load = (GLFunctionLoad)opengl_device->egl_instance.eglGetProcAddress;
	#else
		load = (GLFunctionLoad)opengl_device->egl_instance.eglGetProcAddress;
	#endif

	GLFunction fn = load(OpenGLFunctionIndexToFunctionName[index]);
	if(!fn && index > OPENGL_FUNCTION_INDEX_START_ENUM && !OpenGLLoadFallbackFunction(device, index, load))
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return false;
	}
	opengl_device->original_function_ptrs[index] = fn;
	return true;
}

static bool OpenGLLoadCoreFunctions(PulseDevice device)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	for(int i = 0; i < OPENGL_CORE_FUNCTION_INDEX_END_ENUM; i++)
	{
		if(!OpenGLLoadFunction(device, i))
			return false;
	}

	#define PULSE_OPENGL_FUNCTION(fn, T) opengl_device->fn = PulseOpenGLWrapper_##fn;
		#include "OpenGLFunctions.h"
	#undef PULSE_OPENGL_FUNCTION

	return true;
}

static bool OpenGLLoadFunctions(PulseDevice device)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	for(int i = OPENGL_CORE_FUNCTION_INDEX_END_ENUM; i < OPENGL_FUNCTION_INDEX_END_ENUM; i++)
	{
		if(!OpenGLLoadFunction(device, i))
			return false;
	}

	#undef PULSE_OPENGL_GL_GLES_FUNCTION

	#define PULSE_OPENGL_FUNCTION(fn, T)
	#define PULSE_OPENGL_GL_GLES_FUNCTION(glver, glesver,fn, T) opengl_device->fn = PulseOpenGLWrapper_##fn;
		#include "OpenGLFunctions.h"
	#undef PULSE_OPENGL_FUNCTION
	#undef PULSE_OPENGL_GL_GLES_FUNCTION

	return true;
}

PulseDevice OpenGLCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULLPTR);

	PulseDevice pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);

	OpenGLDevice* device = (OpenGLDevice*)calloc(1, sizeof(OpenGLDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(device, PULSE_NULL_HANDLE);

	pulse_device->driver_data = device;
	pulse_device->backend = backend;

	const char* core_extensions[] = {
	};

	const char* es_extensions[] = {
		"GL_EXT_texture_norm16",
		"GL_EXT_texture_snorm",
	};

	bool is_core = backend->backend == PULSE_BACKEND_OPENGL;

	#ifdef PULSE_PLAT_WINDOWS
		// WGL support
		if(opengl_device->context_type == OPENGL_CONTEXT_WGL)
		{
			// TODO: WGL
			device->context_type = OPENGL_CONTEXT_WGL;
		}
		else
		{
			if(!EGLLoadInstance(&device->egl_instance, is_core ? core_extensions : es_extensions, PULSE_SIZEOF_ARRAY(is_core ? core_extensions : es_extensions), forbiden_devices, forbiden_devices_count, !is_core))
			{
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
					PulseLogError(backend, "could not load EGL instance");
				free(device);
				free(pulse_device);
				return PULSE_NULL_HANDLE;
			}
			device->context_type = OPENGL_CONTEXT_EGL;
		}
	#else
		if(!EGLLoadInstance(&device->egl_instance, is_core ? core_extensions : es_extensions, PULSE_SIZEOF_ARRAY(is_core ? core_extensions : es_extensions), forbiden_devices, forbiden_devices_count, !is_core))
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
				PulseLogError(backend, "could not load EGL instance");
			free(device);
			free(pulse_device);
			return PULSE_NULL_HANDLE;
		}
		device->context_type = OPENGL_CONTEXT_EGL;
	#endif

	if(!OpenGLLoadCoreFunctions(pulse_device))
	{
		EGLUnloadInstance(&device->egl_instance);
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return PULSE_NULL_HANDLE;
	}

	GLint gl_extension_count = 0;
	device->glGetIntegerv(pulse_device, GL_NUM_EXTENSIONS, &gl_extension_count);
	device->supported_extensions_count = gl_extension_count;
	device->supported_extensions = (const char**)calloc(device->supported_extensions_count, sizeof(const char*));
	for(uint32_t i = 0; i < device->supported_extensions_count; i++)
		device->supported_extensions[i] = (const char*)device->glGetStringi(pulse_device, GL_EXTENSIONS, i);

	if(!OpenGLLoadFunctions(pulse_device))
	{
		EGLUnloadInstance(&device->egl_instance);
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return PULSE_NULL_HANDLE;
	}

	if(backend->debug_level > PULSE_LOW_DEBUG && device->original_function_ptrs[glDebugMessageCallback] != PULSE_NULLPTR)
	{
		device->glEnable(pulse_device, GL_DEBUG_OUTPUT);
		//device->glEnable(pulse_device, GL_DEBUG_OUTPUT_SYNCHRONOUS);
		device->glDebugMessageCallback(pulse_device, OpenGLDebugMessageCallback, pulse_device);
		device->glDebugMessageControl(pulse_device, GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, PULSE_NULLPTR, GL_FALSE);
	}

	PULSE_LOAD_DRIVER_DEVICE(OpenGL);

	device->device_id = PulseHashString((const char*)device->glGetString(pulse_device, GL_VENDOR));
	device->device_id = PulseHashCombine(device->device_id, PulseHashString((const char*)device->glGetString(pulse_device, GL_RENDERER)));
	for(uint32_t i = 0; i < device->supported_extensions_count; i++)
		device->device_id = PulseHashCombine(device->device_id, PulseHashString(device->supported_extensions[i]));

	OpenGLInitBindsGroupPoolManager(&device->binds_group_pool_manager, pulse_device);
	OpenGLInitBindsGroupLayoutManager(&device->binds_group_layout_manager, pulse_device);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
		PulseLogInfoFmt(backend, "%s created device from %s", backend->backend == PULSE_BACKEND_OPENGL ? "(OpenGL)" : "(OpenGL ES)", device->glGetString(pulse_device, GL_RENDERER));
	return pulse_device;
}

bool OpenGLDeviceSupportsExtension(PulseDevice device, const char* name)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	for(uint32_t i = 0; i < opengl_device->supported_extensions_count; i++)
	{
		if(strcmp(opengl_device->supported_extensions[i], name) == 0)
			return true;
	}
	return false;
}

void OpenGLDestroyDevice(PulseDevice device)
{
	if(device == PULSE_NULL_HANDLE || device->driver_data == PULSE_NULLPTR)
		return;
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	OpenGLDestroyBindsGroupPoolManager(&opengl_device->binds_group_pool_manager);
	OpenGLDestroyBindsGroupLayoutManager(&opengl_device->binds_group_layout_manager);
	#ifdef PULSE_PLAT_WINDOWS
		if(opengl_device->context_type == OPENGL_CONTEXT_WGL)
		{} // TODO: WGL
		else
			EGLUnloadInstance(&opengl_device->egl_instance);
	#else
		EGLUnloadInstance(&opengl_device->egl_instance);
	#endif
	free(opengl_device);
	free(device);
}
