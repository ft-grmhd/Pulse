// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_FENCE_H_
#define PULSE_WEBGPU_FENCE_H_

#include <webgpu/webgpu.h>
#include <stdatomic.h>

#include <Pulse.h>

typedef struct WebGPUFence
{
	atomic_bool signal;
} WebGPUFence;

PulseFence WebGPUCreateFence(PulseDevice device);
void WebGPUDestroyFence(PulseDevice device, PulseFence fence);
bool WebGPUIsFenceReady(PulseDevice device, PulseFence fence);
bool WebGPUWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all);

#endif // PULSE_WEBGPU_FENCE_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
