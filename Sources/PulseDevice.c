// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

PULSE_API PulseDevice PulseCreateDevice(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used)
{
}

PULSE_API void PulseDestroyDevice(PulseDevice device)
{
}

PULSE_API PulseBackendBits PulseGetBackendInUseByDevice(PulseDevice device)
{
}

PULSE_API bool PulseSupportsBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used)
{
}
