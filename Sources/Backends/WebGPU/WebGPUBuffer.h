// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_BUFFER_H_
#define PULSE_WEBGPU_BUFFER_H_

#include <webgpu/webgpu.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

typedef struct WebGPUBuffer
{
} WebGPUBuffer;

PulseBuffer WebGPUCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos);
bool WebGPUMapBuffer(PulseBuffer buffer, void** data);
void WebGPUUnmapBuffer(PulseBuffer buffer);
bool WebGPUCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst);
bool WebGPUCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst);
void WebGPUDestroyBuffer(PulseDevice device, PulseBuffer buffer);

#endif // PULSE_WEBGPU_BUFFER_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
