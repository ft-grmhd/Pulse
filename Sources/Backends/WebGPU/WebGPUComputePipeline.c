// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "WebGPUComputePipeline.h"

PulseComputePipeline WebGPUCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);

	PulseComputePipelineHandler* pipeline = (PulseComputePipelineHandler*)calloc(1, sizeof(PulseComputePipelineHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pipeline, PULSE_NULL_HANDLE);

	WebGPUComputePipeline* webgpu_pipeline = (WebGPUComputePipeline*)calloc(1, sizeof(WebGPUComputePipeline));
	PULSE_CHECK_ALLOCATION_RETVAL(webgpu_pipeline, PULSE_NULL_HANDLE);

	pipeline->driver_data = webgpu_pipeline;

	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(info->code == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid code pointer passed to PulseComputePipelineCreateInfo");
		if(info->entrypoint == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid entrypoint pointer passed to PulseComputePipelineCreateInfo");
		if(info->format == PULSE_SHADER_FORMAT_WGSL_BIT && (device->backend->supported_shader_formats & PULSE_SHADER_FORMAT_WGSL_BIT) == 0)
			PulseLogError(device->backend, "invalid shader format passed to PulseComputePipelineCreateInfo");
	}

	WGPUStringView code = { 0 };
	code.length = info->code_size;
	code.data = (const char*)info->code;
	WGPUChainedStruct chain = { 0 };
	chain.next = PULSE_NULLPTR;
	chain.sType = WGPUSType_ShaderSourceWGSL;
	WGPUShaderSourceWGSL source = { 0 };
	source.chain = chain;
	source.code = code;
	WGPUShaderModuleDescriptor shader_descriptor = { 0 };
	shader_descriptor.nextInChain = (const WGPUChainedStruct*)&source;
	webgpu_pipeline->shader = wgpuDeviceCreateShaderModule(webgpu_device->device, &shader_descriptor);

	WGPUStringView entrypoint = { 0 };
	code.length = WGPU_STRLEN;
	code.data = info->entrypoint;
	WGPUProgrammableStageDescriptor state = { 0 };
	state.module = webgpu_pipeline->shader;
	state.entryPoint = entrypoint;
	WGPUComputePipelineDescriptor pipeline_descriptor = { 0 };
	pipeline_descriptor.compute = state;
	webgpu_pipeline->pipeline = wgpuDeviceCreateComputePipeline(webgpu_device->device, &pipeline_descriptor);

	return pipeline;
}

void WebGPUDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
}
