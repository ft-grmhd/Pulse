// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_METAL_BACKEND

#ifndef PULSE_METAL_H_
#define PULSE_METAL_H_

PulseBackendFlags MetalCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_METAL in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_METAL_H_

#endif // PULSE_ENABLE_METAL_BACKEND
