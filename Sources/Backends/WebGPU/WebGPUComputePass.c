// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "WebGPUImage.h"
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
	wgpuComputePassEncoderRelease(webgpu_pass->encoder);

	wgpuBindGroupRelease(webgpu_pass->read_only_bind_group);
	wgpuBindGroupRelease(webgpu_pass->read_write_bind_group);
	wgpuBindGroupRelease(webgpu_pass->uniform_bind_group);
	webgpu_pass->read_only_bind_group = PULSE_NULLPTR;
	webgpu_pass->read_write_bind_group = PULSE_NULLPTR;
	webgpu_pass->uniform_bind_group = PULSE_NULLPTR;
}

void WebGPUBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
	PulseBufferUsageFlags usage = buffers[0]->usage;
	bool is_readwrite = (usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) != 0;
	PulseBuffer* array = is_readwrite ? pass->readwrite_storage_buffers : pass->readonly_storage_buffers;
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);

	for(uint32_t i = 0; i < num_buffers; i++)
	{
		if(is_readwrite && (buffers[i]->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) == 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read only buffer with read-write buffers");
			PulseSetInternalError(PULSE_ERROR_INVALID_BUFFER_USAGE);
			return;
		}
		else if(!is_readwrite && (buffers[i]->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) != 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read-write buffer with read only buffers");
			PulseSetInternalError(PULSE_ERROR_INVALID_BUFFER_USAGE);
			return;
		}

		if(array[i] == buffers[i])
			continue;
		array[i] = buffers[i];

		if(is_readwrite)
			webgpu_pass->should_recreate_write_bind_group = true;
		else
			webgpu_pass->should_recreate_read_only_bind_group = true;
	}
}

void WebGPUBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void WebGPUBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
	PulseImageUsageFlags usage = images[0]->usage;
	bool is_readwrite = (usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0;
	PulseImage* array = is_readwrite ? pass->readwrite_images : pass->readonly_images;
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);

	for(uint32_t i = 0; i < num_images; i++)
	{
		if(is_readwrite && (images[i]->usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) == 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read only image with read-write images");
			PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_USAGE);
			return;
		}
		else if(!is_readwrite && (images[i]->usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read-write image with read only images");
			PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_USAGE);
			return;
		}

		if(array[i] == images[i])
			continue;
		array[i] = images[i];
		
		if((usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0)
			webgpu_pass->should_recreate_write_bind_group = true;
		else
			webgpu_pass->should_recreate_read_only_bind_group = true;
	}
}

static void WebGPUBindBindGroups(PulseComputePass pass)
{
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->cmd->device, WebGPUDevice*);
	WebGPUComputePipeline* webgpu_pipeline = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->current_pipeline, WebGPUComputePipeline*);

	if(!webgpu_pass->should_recreate_read_only_bind_group && !webgpu_pass->should_recreate_write_bind_group && !webgpu_pass->should_recreate_uniform_bind_group)
		return;

	WGPUBindGroupEntry* read_only_entries = (WGPUBindGroupEntry*)calloc(pass->current_pipeline->num_readonly_storage_images + pass->current_pipeline->num_readonly_storage_buffers, sizeof(WGPUBindGroupEntry));
	PULSE_CHECK_ALLOCATION(read_only_entries);
	WGPUBindGroupEntry* read_write_entries = (WGPUBindGroupEntry*)calloc(pass->current_pipeline->num_readwrite_storage_images + pass->current_pipeline->num_readwrite_storage_buffers, sizeof(WGPUBindGroupEntry));
	PULSE_CHECK_ALLOCATION(read_write_entries);
	WGPUBindGroupEntry* uniform_entries = (WGPUBindGroupEntry*)calloc(pass->current_pipeline->num_uniform_buffers, sizeof(WGPUBindGroupEntry));
	PULSE_CHECK_ALLOCATION(uniform_entries);

	if(webgpu_pass->should_recreate_read_only_bind_group && webgpu_pipeline->readonly_group != PULSE_NULLPTR)
	{
		uint32_t entry_index = 0;
		for(uint32_t i = 0; i < pass->current_pipeline->num_readonly_storage_images; i++, entry_index++)
		{
			WebGPUImage* webgpu_image = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->readonly_images[i], WebGPUImage*);

			WGPUBindGroupEntry* entry = &read_only_entries[entry_index];
			memset(entry, 0, sizeof(WGPUBindGroupEntry));
			entry->binding = i;
			entry->textureView = webgpu_image->view;
		}

		for(uint32_t i = 0; i < pass->current_pipeline->num_readonly_storage_buffers; i++, entry_index++)
		{
			WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->readonly_storage_buffers[i], WebGPUBuffer*);

			WGPUBindGroupEntry* entry = &read_only_entries[entry_index];
			memset(entry, 0, sizeof(WGPUBindGroupEntry));
			entry->binding = pass->current_pipeline->num_readonly_storage_images + i;
			entry->buffer = webgpu_buffer->buffer;
			entry->size = pass->readonly_storage_buffers[i]->size;
			entry->offset = 0;
		}

		WGPUBindGroupDescriptor descriptor = { 0 };
		descriptor.layout = webgpu_pipeline->readonly_group;
		descriptor.entryCount = pass->current_pipeline->num_readonly_storage_images + pass->current_pipeline->num_readonly_storage_buffers;
		descriptor.entries = read_only_entries;
		webgpu_pass->read_only_bind_group = wgpuDeviceCreateBindGroup(webgpu_device->device, &descriptor);
		wgpuComputePassEncoderSetBindGroup(webgpu_pass->encoder, 0, webgpu_pass->read_only_bind_group, 0, PULSE_NULLPTR);
	}
	if(webgpu_pass->should_recreate_write_bind_group && webgpu_pipeline->readwrite_group != PULSE_NULLPTR)
	{
		uint32_t entry_index = 0;
		for(uint32_t i = 0; i < pass->current_pipeline->num_readwrite_storage_images; i++, entry_index++)
		{
			WebGPUImage* webgpu_image = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->readwrite_images[i], WebGPUImage*);

			WGPUBindGroupEntry* entry = &read_write_entries[entry_index];
			memset(entry, 0, sizeof(WGPUBindGroupEntry));
			entry->binding = i;
			entry->textureView = webgpu_image->view;
		}

		for(uint32_t i = 0; i < pass->current_pipeline->num_readwrite_storage_buffers; i++, entry_index++)
		{
			WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->readwrite_storage_buffers[i], WebGPUBuffer*);

			WGPUBindGroupEntry* entry = &read_write_entries[entry_index];
			memset(entry, 0, sizeof(WGPUBindGroupEntry));
			entry->binding = pass->current_pipeline->num_readwrite_storage_images + i;
			entry->buffer = webgpu_buffer->buffer;
			entry->size = pass->readwrite_storage_buffers[i]->size;
			entry->offset = 0;
		}

		WGPUBindGroupDescriptor descriptor = { 0 };
		descriptor.layout = webgpu_pipeline->readwrite_group;
		descriptor.entryCount = pass->current_pipeline->num_readwrite_storage_images + pass->current_pipeline->num_readwrite_storage_buffers;
		descriptor.entries = read_write_entries;
		webgpu_pass->read_write_bind_group = wgpuDeviceCreateBindGroup(webgpu_device->device, &descriptor);
		wgpuComputePassEncoderSetBindGroup(webgpu_pass->encoder, 1, webgpu_pass->read_write_bind_group, 0, PULSE_NULLPTR);
	}
	if(webgpu_pass->should_recreate_uniform_bind_group && webgpu_pipeline->uniform_group != PULSE_NULLPTR)
	{
		for(uint32_t i = 0; i < pass->current_pipeline->num_uniform_buffers; i++)
		{
			WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass->uniform_buffers[i], WebGPUBuffer*);

			WGPUBindGroupEntry* entry = &uniform_entries[i];
			memset(entry, 0, sizeof(WGPUBindGroupEntry));
			entry->binding = i;
			entry->buffer = webgpu_buffer->buffer;
			entry->size = pass->uniform_buffers[i]->size;
			entry->offset = 0;
		}

		WGPUBindGroupDescriptor descriptor = { 0 };
		descriptor.layout = webgpu_pipeline->uniform_group;
		descriptor.entryCount = pass->current_pipeline->num_uniform_buffers;
		descriptor.entries = uniform_entries;
		webgpu_pass->uniform_bind_group = wgpuDeviceCreateBindGroup(webgpu_device->device, &descriptor);
		wgpuComputePassEncoderSetBindGroup(webgpu_pass->encoder, 2, webgpu_pass->uniform_bind_group, 0, PULSE_NULLPTR);
	}
}

void WebGPUBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);
	WebGPUComputePipeline* webgpu_pipeline = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pipeline, WebGPUComputePipeline*);
	wgpuComputePassEncoderSetPipeline(webgpu_pass->encoder, webgpu_pipeline->pipeline);

	webgpu_pass->should_recreate_read_only_bind_group = true;
	webgpu_pass->should_recreate_write_bind_group = true;
	webgpu_pass->should_recreate_uniform_bind_group = true;
}

void WebGPUDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(pass, WebGPUComputePass*);
	WebGPUBindBindGroups(pass);
	wgpuComputePassEncoderDispatchWorkgroups(webgpu_pass->encoder, groupcount_x, groupcount_y, groupcount_z);
}
