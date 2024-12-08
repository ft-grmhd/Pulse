// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_D3D11_BACKEND

#ifndef PULSE_D3D11_H_
#define PULSE_D3D11_H_

PulseBackendFlags D3D11CheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_VULKAN in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_D3D11_H_

#endif // PULSE_ENABLE_D3D11_BACKEND
