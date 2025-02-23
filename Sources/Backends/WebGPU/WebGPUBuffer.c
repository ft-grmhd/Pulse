// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "WebGPU.h"

PulseBuffer WebGPUCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
}

bool WebGPUMapBuffer(PulseBuffer buffer, void** data)
{
}

void WebGPUUnmapBuffer(PulseBuffer buffer)
{
}

bool WebGPUCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
}

bool WebGPUCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
}

void WebGPUDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
}
