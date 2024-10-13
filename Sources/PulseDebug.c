// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <PulseProfile.h>
#include "PulseDebug.h"

const char* PulseVerbaliseErrorType(PulseErrorType error)
{
	switch(error)
	{
		case PULSE_ERROR_BACKENDS_CANDIDATES_SHADER_FORMAT_MISMATCH: return "no backend candidates support the required shader formats";
		case PULSE_ERROR_INITIALIZATION_FAILED:                      return "initialization of an object could not be completed for implementation-specific reasons";
		case PULSE_ERROR_CPU_ALLOCATION_FAILED:                      return "a CPU allocation failed";
		case PULSE_ERROR_DEVICE_ALLOCATION_FAILED:                   return "a device allocation failed";
		case PULSE_ERROR_DEVICE_LOST:                                return "device has been lost";
		case PULSE_ERROR_INVALID_INTERNAL_POINTER:                   return "invalid internal pointer";
		case PULSE_ERROR_INVALID_HANDLE:                             return "invalid handle";

		default: return "invalid error type";
	};
	return PULSE_NULLPTR; // To avoid warnings, should be unreachable
}

const char* PulseVerbaliseWarningType(PulseWarningType warning)
{
	switch(warning)
	{
		default: return "invalid warning type";
	};
	return PULSE_NULLPTR; // To avoid warnings, should be unreachable
}
