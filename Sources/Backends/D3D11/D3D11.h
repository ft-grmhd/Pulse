// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_D3D11_BACKEND

#ifndef PULSE_D3D11_H_
#define PULSE_D3D11_H_

#ifdef PULSE_PLAT_WINDOWS
	#include <initguid.h>
#endif
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>

#include "../../PulseInternal.h"

#define D3D11_RETRIEVE_DRIVER_DATA_AS(handle, cast) ((cast)handle->driver_data)

PulseBackendFlags Direct3D11CheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_D3D11 in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_D3D11_H_

#endif // PULSE_ENABLE_D3D11_BACKEND
