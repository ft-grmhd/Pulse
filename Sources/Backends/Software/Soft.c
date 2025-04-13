// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <cpuinfo.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

#if defined(__STDC_NO_ATOMICS__) && !defined(ATOMICS_OVERLOAD)
	#error "Atomic support is not present"
#endif

#include "Soft.h"
#include "SoftDevice.h"

PulseBackendFlags SoftCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_SOFTWARE) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_SPIRV_BIT) == 0)
		return PULSE_BACKEND_INVALID;
	return PULSE_BACKEND_SOFTWARE; // Software computer is always supported
}

bool SoftLoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	PULSE_UNUSED(backend);
	PULSE_UNUSED(debug_level);
	cpuinfo_initialize();
	return true;
}

void SoftUnloadBackend(PulseBackend backend)
{
	cpuinfo_deinitialize();
	free(backend->driver_data);
}

PulseBackendHandler SoftwareDriver = {
	.PFN_LoadBackend = SoftLoadBackend,
	.PFN_UnloadBackend = SoftUnloadBackend,
	.PFN_CreateDevice = SoftCreateDevice,
	.backend = PULSE_BACKEND_SOFTWARE,
	.supported_shader_formats = PULSE_SHADER_FORMAT_SPIRV_BIT,
	.driver_data = PULSE_NULLPTR
};
