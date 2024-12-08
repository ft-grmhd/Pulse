// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "D3D11.h"

PulseBackendFlags D3D11CheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_D3D11) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_DXBC_BIT) == 0)
		return PULSE_BACKEND_INVALID;
	return PULSE_BACKEND_INVALID; // Not supported
}

PulseBackendHandler D3D11Driver = {
	.PFN_LoadBackend = PULSE_NULLPTR,
	.PFN_UnloadBackend = PULSE_NULLPTR,
	.PFN_CreateDevice = PULSE_NULLPTR,
	.backend = PULSE_BACKEND_D3D11,
	.supported_shader_formats = PULSE_SHADER_FORMAT_DXBC_BIT,
	.driver_data = PULSE_NULLPTR
};
