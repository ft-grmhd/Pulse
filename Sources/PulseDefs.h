// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_DEFS_H_
#define PULSE_DEFS_H_

#include <Pulse.h>

#define PulseStaticAllocStack(size) ((char[size]){ 0 })

#define PULSE_CHECK_ALLOCATION_RETVAL(ptr, retval) \
	do { \
		if(ptr == PULSE_NULLPTR) \
		{ \
			PulseSetInternalError(PULSE_ERROR_CPU_ALLOCATION_FAILED); \
			return retval; \
		} \
	} while(0); \

#define PULSE_CHECK_ALLOCATION(ptr) PULSE_CHECK_ALLOCATION_RETVAL(ptr, )

#define PULSE_CHECK_HANDLE_RETVAL(handle, retval) \
	do { \
		if(handle == PULSE_NULL_HANDLE) \
		{ \
			PulseSetInternalError(PULSE_ERROR_INVALID_HANDLE); \
			return retval; \
		} \
	} while(0); \

#define PULSE_CHECK_HANDLE(handle) PULSE_CHECK_HANDLE_RETVAL(handle, )

#define PULSE_CHECK_PTR_RETVAL(ptr, retval) \
	do { \
		if(ptr == PULSE_NULLPTR) \
		{ \
			PulseSetInternalError(PULSE_ERROR_INVALID_INTERNAL_POINTER); \
			return retval; \
		} \
	} while(0); \

#define PULSE_UNUSED(x) ((void)x)

#define PULSE_CHECK_BACKEND_DEBUG_LEVEL(backend, level) (backend != PULSE_NULL_HANDLE && ((PulseBackend)backend)->debug_level >= level)

#define PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend) PULSE_CHECK_BACKEND_DEBUG_LEVEL(backend, PULSE_LOW_DEBUG)
#define PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend) PULSE_CHECK_BACKEND_DEBUG_LEVEL(backend, PULSE_HIGH_DEBUG)

#define PULSE_CHECK_PTR(handle) PULSE_CHECK_PTR_RETVAL(handle, )

#define PULSE_EXPAND_ARRAY_IF_NEEDED(array, T, size, capacity, increase) \
	do { \
		if(size >= capacity) \
		{ \
			capacity += increase; \
			array = (T*)realloc(array, sizeof(T) * capacity); \
		} \
	} while(0); \

#define PULSE_DEFRAG_ARRAY(array, size, start) \
	for(size_t defrag_i = start; defrag_i < size - 1; defrag_i++) \
		array[defrag_i] = array[defrag_i + 1]; \

#ifndef PULSE_STATIC_ASSERT
	#ifdef __cplusplus
		#if __cplusplus >= 201103L
			#define PULSE_STATIC_ASSERT(name, x)  static_assert(x, #x)
		#endif
	#elif PULSE_C_VERSION >= 2023
		#define PULSE_STATIC_ASSERT(name, x)  static_assert(x, #x)
	#elif PULSE_C_VERSION >= 2011
		#define PULSE_STATIC_ASSERT(name, x) _Static_assert(x, #x)
	#else
		#define PULSE_STATIC_ASSERT(name, x) typedef int pulse_static_assert_##name[(x) ? 1 : -1]
	#endif
#endif

#define PULSE_LOAD_DRIVER_DEVICE_FUNCTION(fn, _namespace) pulse_device->PFN_##fn = _namespace##fn;
#define PULSE_LOAD_DRIVER_DEVICE(_namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyDevice, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DispatchComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateFence, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyFence, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(IsFenceReady, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(WaitForFences, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(RequestCommandList, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(SubmitCommandList, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(ReleaseCommandList, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateBuffer, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(MapBuffer, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(UnmapBuffer, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyBuffer, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateImage, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(IsImageFormatValid, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyImage, _namespace) \

#endif // PULSE_DEFS_H_
