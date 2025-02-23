// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "WebGPU.h"

PulseFence WebGPUCreateFence(PulseDevice device)
{
}

void WebGPUDestroyFence(PulseDevice device, PulseFence fence)
{
}

bool WebGPUIsFenceReady(PulseDevice device, PulseFence fence)
{
}

bool WebGPUWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
}
