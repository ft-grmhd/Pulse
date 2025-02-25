// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_COMPUTE_PIPELINE_H_
#define PULSE_WEBGPU_COMPUTE_PIPELINE_H_

#include <webgpu/webgpu.h>

#include <Pulse.h>

typedef struct WebGPUComputePipeline
{
	WGPUComputePipeline pipeline;
	WGPUShaderModule shader;
} WebGPUComputePipeline;

PulseComputePipeline WebGPUCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info);
void WebGPUDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline);

#endif // PULSE_WEBGPU_COMPUTE_PIPELINE_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
