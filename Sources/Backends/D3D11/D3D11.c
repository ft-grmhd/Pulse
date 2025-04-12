// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "D3D11.h"
#include "D3D11Device.h"

PulseBackendFlags PuD3D11CheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_D3D11) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_DXBC_BIT) == 0)
		return PULSE_BACKEND_INVALID;
	return PULSE_BACKEND_D3D11;
}

bool PuD3D11LoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	return true;
}

void PuD3D11UnloadBackend(PulseBackend backend)
{
}

PulseBackendHandler D3D11Driver = {
	.PFN_LoadBackend = PuD3D11LoadBackend,
	.PFN_UnloadBackend = PuD3D11UnloadBackend,
	.PFN_CreateDevice = PuD3D11CreateDevice,
	.backend = PULSE_BACKEND_D3D11,
	.supported_shader_formats = PULSE_SHADER_FORMAT_DXBC_BIT,
	.driver_data = PULSE_NULLPTR
};

