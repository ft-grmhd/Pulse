// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_COMMAND_LIST_H_
#define PULSE_WEBGPU_COMMAND_LIST_H_

#include <webgpu/webgpu.h>

#include <Pulse.h>
#include "WebGPUBuffer.h"

typedef struct WebGPUCommandList
{
	WGPUCommandEncoder encoder;
} WebGPUCommandList;

PulseCommandList WebGPURequestCommandList(PulseDevice device, PulseCommandListUsage usage);
bool WebGPUSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
void WebGPUReleaseCommandList(PulseDevice device, PulseCommandList cmd);

#endif // PULSE_WEBGPU_COMMAND_LIST_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
