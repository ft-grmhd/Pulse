// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "webgpu.h"
#include "WebGPUComputePipeline.h"

static WGPUBindGroupLayout WebGPUCreateBindGroupLayout(PulseDevice device,
														uint32_t read_storage_images_count,
														uint32_t read_storage_buffers_count,
														uint32_t write_storage_images_count,
														uint32_t write_storage_buffers_count,
														uint32_t uniform_buffers_count)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);

	uint8_t category;
	if(uniform_buffers_count != 0)
		category = 3;
	else if(write_storage_images_count != 0 || write_storage_buffers_count != 0)
		category = 2;
	else
		category = 1;

	uint32_t count = 0;
	WGPUBindGroupLayoutEntry entries[PULSE_MAX_READ_BUFFERS_BOUND + PULSE_MAX_READ_TEXTURES_BOUND + PULSE_MAX_WRITE_BUFFERS_BOUND + PULSE_MAX_WRITE_TEXTURES_BOUND + PULSE_MAX_UNIFORM_BUFFERS_BOUND] = { 0 };

	if(category == 1)
	{
		for(uint32_t i = 0; i < read_storage_images_count; i++, count++)
		{
			entries[i].binding = i;
			entries[i].visibility = WGPUShaderStage_Compute;

			entries[i].buffer.nextInChain = PULSE_NULLPTR;
			entries[i].buffer.hasDynamicOffset = false;
			entries[i].buffer.type = WGPUBufferBindingType_BindingNotUsed;
			entries[i].buffer.minBindingSize = 0;

			entries[i].sampler.nextInChain = PULSE_NULLPTR;
			entries[i].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[i].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[i].storageTexture.access = WGPUStorageTextureAccess_Undefined;
			entries[i].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[i].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[i].texture.nextInChain = PULSE_NULLPTR;
			entries[i].texture.multisampled = false;
			entries[i].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[i].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}

		for(uint32_t i = read_storage_images_count; i < read_storage_images_count + read_storage_buffers_count; i++, count++)
		{
			entries[i].binding = i;
			entries[i].visibility = WGPUShaderStage_Compute;

			entries[i].buffer.nextInChain = PULSE_NULLPTR;
			entries[i].buffer.hasDynamicOffset = false;
			entries[i].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
			entries[i].buffer.minBindingSize = 0;

			entries[i].sampler.nextInChain = PULSE_NULLPTR;
			entries[i].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[i].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[i].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[i].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[i].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[i].texture.nextInChain = PULSE_NULLPTR;
			entries[i].texture.multisampled = false;
			entries[i].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[i].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}
	}
	else if(category == 2)
	{
		for(uint32_t i = 0; i < write_storage_images_count; i++, count++)
		{
			entries[i].binding = i;
			entries[i].visibility = WGPUShaderStage_Compute;

			entries[i].buffer.nextInChain = PULSE_NULLPTR;
			entries[i].buffer.hasDynamicOffset = false;
			entries[i].buffer.type = WGPUBufferBindingType_BindingNotUsed;
			entries[i].buffer.minBindingSize = 0;

			entries[i].sampler.nextInChain = PULSE_NULLPTR;
			entries[i].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[i].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[i].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[i].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[i].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[i].texture.nextInChain = PULSE_NULLPTR;
			entries[i].texture.multisampled = false;
			entries[i].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[i].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}

		for(uint32_t i = write_storage_images_count; i < write_storage_images_count + write_storage_buffers_count; i++, count++)
		{
			entries[i].binding = i;
			entries[i].visibility = WGPUShaderStage_Compute;

			entries[i].buffer.nextInChain = PULSE_NULLPTR;
			entries[i].buffer.hasDynamicOffset = false;
			entries[i].buffer.type = WGPUBufferBindingType_Storage;
			entries[i].buffer.minBindingSize = 0;

			entries[i].sampler.nextInChain = PULSE_NULLPTR;
			entries[i].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[i].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[i].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[i].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[i].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[i].texture.nextInChain = PULSE_NULLPTR;
			entries[i].texture.multisampled = false;
			entries[i].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[i].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}
	}
	else if(category == 3)
	{
		for(uint32_t i = 0; i < uniform_buffers_count; i++, count++)
		{
			entries[i].binding = i;
			entries[i].visibility = WGPUShaderStage_Compute;

			entries[i].buffer.nextInChain = PULSE_NULLPTR;
			entries[i].buffer.hasDynamicOffset = false;
			entries[i].buffer.type = WGPUBufferBindingType_Uniform;
			entries[i].buffer.minBindingSize = 0;

			entries[i].sampler.nextInChain = PULSE_NULLPTR;
			entries[i].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[i].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[i].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[i].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[i].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[i].texture.nextInChain = PULSE_NULLPTR;
			entries[i].texture.multisampled = false;
			entries[i].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[i].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}
	}

	WGPUBindGroupLayoutDescriptor descriptor = { 0 };
	descriptor.entryCount = count;
	descriptor.entries = entries;
	return wgpuDeviceCreateBindGroupLayout(webgpu_device->device, &descriptor);
}

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

	WGPUShaderSourceWGSL source = { 0 };
	source.chain.next = PULSE_NULLPTR;
	source.chain.sType = WGPUSType_ShaderSourceWGSL;
	source.code.length = info->code_size;
	source.code.data = (const char*)info->code;

	WGPUShaderModuleDescriptor shader_descriptor = { 0 };
	shader_descriptor.nextInChain = (const WGPUChainedStruct*)&source;
	webgpu_pipeline->shader = wgpuDeviceCreateShaderModule(webgpu_device->device, &shader_descriptor);

	webgpu_pipeline->readonly_group  = WebGPUCreateBindGroupLayout(device, info->num_readonly_storage_images, info->num_readonly_storage_buffers, 0, 0, 0);
	webgpu_pipeline->readwrite_group = WebGPUCreateBindGroupLayout(device, 0, 0, info->num_readwrite_storage_images, info->num_readwrite_storage_buffers, 0);
	webgpu_pipeline->uniform_group   = WebGPUCreateBindGroupLayout(device, 0, 0, 0, 0, info->num_uniform_buffers);

	WGPUBindGroupLayout bind_group_layouts[3] = {
		webgpu_pipeline->readonly_group,
		webgpu_pipeline->readwrite_group,
		webgpu_pipeline->uniform_group,
	};

	WGPUPipelineLayoutDescriptor layout_descriptor = { 0 };
	layout_descriptor.bindGroupLayoutCount = 3;
	layout_descriptor.bindGroupLayouts = bind_group_layouts;
	webgpu_pipeline->layout = wgpuDeviceCreatePipelineLayout(webgpu_device->device, &layout_descriptor);

	WGPUComputePipelineDescriptor pipeline_descriptor = { 0 };
	pipeline_descriptor.compute.module = webgpu_pipeline->shader;
	pipeline_descriptor.compute.entryPoint.length = WGPU_STRLEN;
	pipeline_descriptor.compute.entryPoint.data = info->entrypoint;
	pipeline_descriptor.layout = webgpu_pipeline->layout;
	webgpu_pipeline->pipeline = wgpuDeviceCreateComputePipeline(webgpu_device->device, &pipeline_descriptor);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(WebGPU) created new compute pipeline %p", pipeline);
	return pipeline;
}

void WebGPUDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	if(pipeline == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "compute pipeline is NULL, this may be a bug in your application");
		return;
	}

	PULSE_UNUSED(device);
	WebGPUComputePipeline* webgpu_pipeline = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pipeline, WebGPUComputePipeline*);
	wgpuBindGroupLayoutRelease(webgpu_pipeline->readonly_group);
	wgpuBindGroupLayoutRelease(webgpu_pipeline->readwrite_group);
	wgpuBindGroupLayoutRelease(webgpu_pipeline->uniform_group);
	wgpuPipelineLayoutRelease(webgpu_pipeline->layout);
	wgpuComputePipelineRelease(webgpu_pipeline->pipeline);
	wgpuShaderModuleRelease(webgpu_pipeline->shader);
	free(webgpu_pipeline);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(WebGPU) destroyed compute pipeline %p", pipeline);

	free(pipeline);
}
