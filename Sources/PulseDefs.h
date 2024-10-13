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

#define PULSE_CHECK_PTR(handle) PULSE_CHECK_PTR_RETVAL(handle, )

#define PULSE_LOAD_DRIVER_DEVICE_FUNCTION(fn, _namespace) pulse_device->PFN_##fn = _namespace##fn;
#define PULSE_LOAD_DRIVER_DEVICE(_namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyDevice, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(BindComputePipeline, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(CreateFence, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(DestroyFence, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(IsFenceReady, _namespace) \
	PULSE_LOAD_DRIVER_DEVICE_FUNCTION(WaitForFences, _namespace) \

#endif // PULSE_DEFS_H_
