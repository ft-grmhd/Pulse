// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "WebGPUComputePass.h"
#include "WebGPUComputePipeline.h"

PulseComputePass WebGPUCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PULSE_UNUSED(device);
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	WebGPUComputePass* webgpu_pass = (WebGPUComputePass*)calloc(1, sizeof(WebGPUComputePass));
	PULSE_CHECK_ALLOCATION_RETVAL(webgpu_pass, PULSE_NULL_HANDLE);

	pass->cmd = cmd;
	pass->driver_data = webgpu_pass;

	return pass;
}

void WebGPUDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
	PULSE_UNUSED(device);
	free(pass->driver_data);
	free(pass);
}

PulseComputePass WebGPUBeginComputePass(PulseCommandList cmd)
{
	WebGPUCommandList* webgpu_cmd = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd, WebGPUCommandList*);
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd->pass, WebGPUComputePass*);
	WGPUComputePassDescriptor descriptor = { 0 };
	webgpu_pass->encoder = wgpuCommandEncoderBeginComputePass(webgpu_cmd->encoder, &descriptor);
	return cmd->pass;
}

void WebGPUEndComputePass(PulseComputePass pass)
{
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);
	wgpuComputePassEncoderEnd(webgpu_pass->encoder);
}

void WebGPUBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
}

void WebGPUBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void WebGPUBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
}

void WebGPUBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);
	WebGPUComputePipeline* webgpu_pipeline = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pipeline, WebGPUComputePipeline*);
	wgpuComputePassEncoderSetPipeline(webgpu_pass->encoder, webgpu_pipeline->pipeline);
}

void WebGPUDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);
	wgpuComputePassEncoderDispatchWorkgroups(webgpu_pass->encoder, groupcount_x, groupcount_y, groupcount_z);
}
