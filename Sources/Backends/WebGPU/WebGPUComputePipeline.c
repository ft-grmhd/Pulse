// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "webgpu.h"
#include "WebGPUComputePipeline.h"

typedef struct WebGPUBindGroupLayoutEntryInfo
{
	WGPUStorageTextureAccess read_access[PULSE_MAX_READ_TEXTURES_BOUND];
	WGPUTextureViewDimension read_dimensions[PULSE_MAX_READ_TEXTURES_BOUND];
	WGPUTextureFormat read_formats[PULSE_MAX_READ_TEXTURES_BOUND];

	WGPUStorageTextureAccess write_access[PULSE_MAX_WRITE_TEXTURES_BOUND];
	WGPUTextureViewDimension write_dimensions[PULSE_MAX_WRITE_TEXTURES_BOUND];
	WGPUTextureFormat write_formats[PULSE_MAX_WRITE_TEXTURES_BOUND];
} WebGPUBindGroupLayoutEntryInfo;

static WGPUTextureFormat WebGPUMapFormat(const char* format)
{
	if(strcmp(format, "r32float") == 0)
		return WGPUTextureFormat_R32Float;
	if(strcmp(format, "rg32float") == 0)
		return WGPUTextureFormat_RG32Float;
	if(strcmp(format, "rgba32float") == 0)
		return WGPUTextureFormat_RGBA32Float;
	if(strcmp(format, "r32uint") == 0)
		return WGPUTextureFormat_R32Uint;
	if(strcmp(format, "r32sint") == 0)
		return WGPUTextureFormat_R32Sint;
	if(strcmp(format, "rg32uint") == 0)
		return WGPUTextureFormat_RG32Uint;
	if(strcmp(format, "rg32sint") == 0)
		return WGPUTextureFormat_RG32Sint;
	if(strcmp(format, "rgba32uint") == 0)
		return WGPUTextureFormat_RGBA32Uint;
	if(strcmp(format, "rgba32sint") == 0)
		return WGPUTextureFormat_RGBA32Sint;
	if(strcmp(format, "rgba8unorm") == 0)
		return WGPUTextureFormat_RGBA8Unorm;
	if(strcmp(format, "rgba8snorm") == 0)
		return WGPUTextureFormat_RGBA8Snorm;
	if(strcmp(format, "rgba8uint") == 0)
		return WGPUTextureFormat_RGBA8Uint;
	if(strcmp(format, "rgba8sint") == 0)
		return WGPUTextureFormat_RGBA8Sint;
	if(strcmp(format, "rg8unorm") == 0)
		return WGPUTextureFormat_RG8Unorm;
	if(strcmp(format, "rg8snorm") == 0)
		return WGPUTextureFormat_RG8Snorm;
	if(strcmp(format, "rg8uint") == 0)
		return WGPUTextureFormat_RG8Uint;
	if(strcmp(format, "rg8sint") == 0)
		return WGPUTextureFormat_RG8Sint;
	if(strcmp(format, "r8unorm") == 0)
		return WGPUTextureFormat_R8Unorm;
	if(strcmp(format, "r8snorm") == 0)
		return WGPUTextureFormat_R8Snorm;
	if(strcmp(format, "r8uint") == 0)
		return WGPUTextureFormat_R8Uint;
	if(strcmp(format, "r8sint") == 0)
		return WGPUTextureFormat_R8Sint;
	if(strcmp(format, "bgra8unorm") == 0)
		return WGPUTextureFormat_BGRA8Unorm;
	return WGPUTextureFormat_Undefined;
}

static WGPUStorageTextureAccess WebGPUMapAccess(const char* access)
{
	if(strcmp(access, "read") == 0)
		return WGPUStorageTextureAccess_ReadOnly;
	if(strcmp(access, "write") == 0)
		return WGPUStorageTextureAccess_WriteOnly;
	if(strcmp(access, "read_write") == 0)
		return WGPUStorageTextureAccess_ReadWrite;
	return WGPUStorageTextureAccess_Undefined;
}

static void WebGPUReflectWGSLBindings(WebGPUBindGroupLayoutEntryInfo* infos, const char* wgsl)
{
	bool found;

	uint32_t count_read_images = 0;
	uint32_t count_write_images = 0;

	char* token;
	char* copy = calloc(strlen(wgsl) + 1, 1);
	PulseStrlcpy(copy, wgsl, strlen(wgsl));

	while((token = PulseStrtokR(copy, "\n", &copy)))
	{
		found = false;
		char* storage_pos;

		WGPUStorageTextureAccess webgpu_access = WGPUStorageTextureAccess_Undefined;
		WGPUTextureFormat webgpu_format = WGPUTextureFormat_Undefined;
		WGPUTextureViewDimension webgpu_dimension = WGPUTextureViewDimension_Undefined;

		if((storage_pos = strstr(token, "texture_storage_2d<")))
		{
			char format[32];
			char access[32];
			if(sscanf(storage_pos, "texture_storage_2d<%[^,],%[^>]>", format, access) == 2)
			{
				PulseTrimString(format);
				PulseTrimString(access);
				webgpu_access = WebGPUMapAccess(access);
				webgpu_format = WebGPUMapFormat(format);
				webgpu_dimension = WGPUTextureViewDimension_2D;
			}
		}
		else if((storage_pos = strstr(token, "texture_storage_2d_array<")))
		{
			char format[32];
			char access[32];
			if(sscanf(storage_pos, "texture_storage_2d_array<%[^,],%[^>]>", format, access) == 2)
			{
				PulseTrimString(format);
				PulseTrimString(access);
				webgpu_access = WebGPUMapAccess(access);
				webgpu_format = WebGPUMapFormat(format);
				webgpu_dimension = WGPUTextureViewDimension_2DArray;
			}
		}
		else if((storage_pos = strstr(token, "texture_storage_3d<")))
		{
			char format[32];
			char access[32];
			if(sscanf(storage_pos, "texture_storage_3d<%[^,],%[^>]>", format, access) == 2)
			{
				PulseTrimString(format);
				PulseTrimString(access);
				webgpu_access = WebGPUMapAccess(access);
				webgpu_format = WebGPUMapFormat(format);
				webgpu_dimension = WGPUTextureViewDimension_3D;
			}
		}

		if(webgpu_access == WGPUStorageTextureAccess_ReadOnly)
		{
			infos->read_access[count_read_images] = webgpu_access;
			infos->read_formats[count_read_images] = webgpu_format;
			infos->read_dimensions[count_read_images] = webgpu_dimension;
			count_read_images++;
		}
		else
		{
			infos->write_access[count_write_images] = webgpu_access;
			infos->write_formats[count_write_images] = webgpu_format;
			infos->write_dimensions[count_write_images] = webgpu_dimension;
			count_write_images++;
		}
	}

	free(copy);
}

static WGPUBindGroupLayout WebGPUCreateBindGroupLayout(PulseDevice device, const WebGPUBindGroupLayoutEntryInfo* infos,
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
			entries[count].binding = count;
			entries[count].visibility = WGPUShaderStage_Compute;

			entries[count].buffer.nextInChain = PULSE_NULLPTR;
			entries[count].buffer.hasDynamicOffset = false;
			entries[count].buffer.type = WGPUBufferBindingType_BindingNotUsed;
			entries[count].buffer.minBindingSize = 0;

			entries[count].sampler.nextInChain = PULSE_NULLPTR;
			entries[count].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[count].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[count].storageTexture.access = infos->read_access[i];
			entries[count].storageTexture.format = infos->read_formats[i];
			entries[count].storageTexture.viewDimension = infos->read_dimensions[i];

			entries[count].texture.nextInChain = PULSE_NULLPTR;
			entries[count].texture.multisampled = false;
			entries[count].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[count].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}

		for(uint32_t i = 0; i < read_storage_buffers_count; i++, count++)
		{
			entries[count].binding = count;
			entries[count].visibility = WGPUShaderStage_Compute;

			entries[count].buffer.nextInChain = PULSE_NULLPTR;
			entries[count].buffer.hasDynamicOffset = false;
			entries[count].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
			entries[count].buffer.minBindingSize = 0;

			entries[count].sampler.nextInChain = PULSE_NULLPTR;
			entries[count].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[count].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[count].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[count].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[count].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[count].texture.nextInChain = PULSE_NULLPTR;
			entries[count].texture.multisampled = false;
			entries[count].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[count].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}
	}
	else if(category == 2)
	{
		for(uint32_t i = 0; i < write_storage_images_count; i++, count++)
		{
			entries[count].binding = count;
			entries[count].visibility = WGPUShaderStage_Compute;

			entries[count].buffer.nextInChain = PULSE_NULLPTR;
			entries[count].buffer.hasDynamicOffset = false;
			entries[count].buffer.type = WGPUBufferBindingType_BindingNotUsed;
			entries[count].buffer.minBindingSize = 0;

			entries[count].sampler.nextInChain = PULSE_NULLPTR;
			entries[count].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[count].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[count].storageTexture.access = infos->write_access[i];
			entries[count].storageTexture.format = infos->write_formats[i];
			entries[count].storageTexture.viewDimension = infos->write_dimensions[i];

			entries[count].texture.nextInChain = PULSE_NULLPTR;
			entries[count].texture.multisampled = false;
			entries[count].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[count].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}

		for(uint32_t i = 0; i < write_storage_buffers_count; i++, count++)
		{
			entries[count].binding = count;
			entries[count].visibility = WGPUShaderStage_Compute;

			entries[count].buffer.nextInChain = PULSE_NULLPTR;
			entries[count].buffer.hasDynamicOffset = false;
			entries[count].buffer.type = WGPUBufferBindingType_Storage;
			entries[count].buffer.minBindingSize = 0;

			entries[count].sampler.nextInChain = PULSE_NULLPTR;
			entries[count].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[count].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[count].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[count].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[count].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[count].texture.nextInChain = PULSE_NULLPTR;
			entries[count].texture.multisampled = false;
			entries[count].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[count].texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}
	}
	else if(category == 3)
	{
		for(uint32_t i = 0; i < uniform_buffers_count; i++, count++)
		{
			entries[count].binding = count;
			entries[count].visibility = WGPUShaderStage_Compute;

			entries[count].buffer.nextInChain = PULSE_NULLPTR;
			entries[count].buffer.hasDynamicOffset = false;
			entries[count].buffer.type = WGPUBufferBindingType_Uniform;
			entries[count].buffer.minBindingSize = 0;

			entries[count].sampler.nextInChain = PULSE_NULLPTR;
			entries[count].sampler.type = WGPUSamplerBindingType_BindingNotUsed;

			entries[count].storageTexture.nextInChain = PULSE_NULLPTR;
			entries[count].storageTexture.access = WGPUStorageTextureAccess_BindingNotUsed;
			entries[count].storageTexture.format = WGPUTextureFormat_Undefined;
			entries[count].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			entries[count].texture.nextInChain = PULSE_NULLPTR;
			entries[count].texture.multisampled = false;
			entries[count].texture.sampleType = WGPUTextureSampleType_BindingNotUsed;
			entries[count].texture.viewDimension = WGPUTextureViewDimension_Undefined;
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

	WebGPUBindGroupLayoutEntryInfo bindgroup_infos = { 0 };

	WebGPUReflectWGSLBindings(&bindgroup_infos, (const char*)info->code);

	webgpu_pipeline->readonly_group  = WebGPUCreateBindGroupLayout(device, &bindgroup_infos, info->num_readonly_storage_images, info->num_readonly_storage_buffers, 0, 0, 0);
	webgpu_pipeline->readwrite_group = WebGPUCreateBindGroupLayout(device, &bindgroup_infos, 0, 0, info->num_readwrite_storage_images, info->num_readwrite_storage_buffers, 0);
	webgpu_pipeline->uniform_group   = WebGPUCreateBindGroupLayout(device, &bindgroup_infos, 0, 0, 0, 0, info->num_uniform_buffers);

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
