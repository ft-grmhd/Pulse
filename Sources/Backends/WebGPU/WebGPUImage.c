// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "WebGPU.h"

PulseImage WebGPUCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos)
{
}

bool WebGPUIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage)
{
}

bool WebGPUCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst)
{
}

bool WebGPUBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst)
{
}

void WebGPUDestroyImage(PulseDevice device, PulseImage image)
{
}
