// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_COMPUTE_PASS_H_
#define PULSE_WEBGPU_COMPUTE_PASS_H_

#include <webgpu/webgpu.h>

#include <Pulse.h>
#include "WebGPUBuffer.h"
#include "WebGPUCommandList.h"

typedef struct WebGPUComputePass
{
	WGPUComputePassEncoder encoder;
} WebGPUComputePass;

PulseComputePass WebGPUCreateComputePass(PulseDevice device, PulseCommandList cmd);
void WebGPUDestroyComputePass(PulseDevice device, PulseComputePass pass);

PulseComputePass WebGPUBeginComputePass(PulseCommandList cmd);
void WebGPUEndComputePass(PulseComputePass pass);
void WebGPUBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers);
void WebGPUBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size);
void WebGPUBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images);
void WebGPUBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline);
void WebGPUDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);

#endif // PULSE_WEBGPU_COMPUTE_PASS_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
