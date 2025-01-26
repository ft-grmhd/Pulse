// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "Metal.h"

PulseBackendFlags MetalCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_METAL) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_MSL_BIT) == 0 && (shader_formats_used & PULSE_SHADER_FORMAT_METALLIB_BIT) == 0)
		return PULSE_BACKEND_INVALID;
	return PULSE_BACKEND_INVALID; // Not supported yet
}

PulseBackendHandler MetalDriver = {
	.PFN_LoadBackend = PULSE_NULLPTR,
	.PFN_UnloadBackend = PULSE_NULLPTR,
	.PFN_CreateDevice = PULSE_NULLPTR,
	.backend = PULSE_BACKEND_METAL,
	.supported_shader_formats = PULSE_SHADER_FORMAT_MSL_BIT | PULSE_SHADER_FORMAT_METALLIB_BIT,
	.driver_data = PULSE_NULLPTR
};
