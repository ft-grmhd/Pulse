// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_IMAGE_H_
#define PULSE_WEBGPU_IMAGE_H_

#include <webgpu/webgpu.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

typedef struct WebGPUImage
{
	WGPUTexture texture;
} WebGPUImage;

PulseImage WebGPUCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos);
bool WebGPUIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage);
bool WebGPUCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst);
bool WebGPUBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst);
void WebGPUDestroyImage(PulseDevice device, PulseImage image);

#endif // PULSE_WEBGPU_IMAGE_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
