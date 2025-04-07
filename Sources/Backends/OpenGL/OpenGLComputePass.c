// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLDevice.h"
#include "OpenGLComputePass.h"
#include "OpenGLComputePipeline.h"

PulseComputePass OpenGLCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PULSE_UNUSED(device);
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	OpenGLComputePass* opengl_pass = (OpenGLComputePass*)calloc(1, sizeof(OpenGLComputePass));
	PULSE_CHECK_ALLOCATION_RETVAL(opengl_pass, PULSE_NULL_HANDLE);

	pass->cmd = cmd;
	pass->driver_data = opengl_pass;

	return pass;
}

void OpenGLDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
	PULSE_UNUSED(device);
	free(pass->driver_data);
	free(pass);
}

PulseComputePass OpenGLBeginComputePass(PulseCommandList cmd)
{
	return cmd->pass;
}

void OpenGLEndComputePass(PulseComputePass pass)
{
	OpenGLComputePass* opengl_pass = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass, OpenGLComputePass*);
	OpenGLReturnBindsGroupToPool(opengl_pass->read_only_bind_group->pool, opengl_pass->read_only_bind_group);
	OpenGLReturnBindsGroupToPool(opengl_pass->read_write_bind_group->pool, opengl_pass->read_write_bind_group);
	OpenGLReturnBindsGroupToPool(opengl_pass->uniform_bind_group->pool, opengl_pass->uniform_bind_group);
	opengl_pass->read_only_bind_group = PULSE_NULLPTR;
	opengl_pass->read_write_bind_group = PULSE_NULLPTR;
	opengl_pass->uniform_bind_group = PULSE_NULLPTR;
}

void OpenGLBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
	PulseBufferUsageFlags usage = buffers[0]->usage;
	bool is_readwrite = (usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) != 0;
	PulseBuffer* array = is_readwrite ? pass->readwrite_storage_buffers : pass->readonly_storage_buffers;
	OpenGLComputePass* opengl_pass = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass, OpenGLComputePass*);

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
			opengl_pass->should_recreate_write_bind_group = true;
		else
			opengl_pass->should_recreate_read_only_bind_group = true;
	}
}

void OpenGLBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void OpenGLBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
	PulseImageUsageFlags usage = images[0]->usage;
	bool is_readwrite = (usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0;
	PulseImage* array = is_readwrite ? pass->readwrite_images : pass->readonly_images;
	OpenGLComputePass* opengl_pass = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass, OpenGLComputePass*);

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
		
		if(is_readwrite)
			opengl_pass->should_recreate_write_bind_group = true;
		else
			opengl_pass->should_recreate_read_only_bind_group = true;
	}
}

static void OpenGLBindBindsGroup(PulseComputePass pass)
{
	OpenGLComputePass* opengl_pass = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass, OpenGLComputePass*);
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass->cmd->device, OpenGLDevice*);
	OpenGLComputePipeline* opengl_pipeline = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass->current_pipeline, OpenGLComputePipeline*);

	if(!opengl_pass->should_recreate_read_only_bind_group && !opengl_pass->should_recreate_write_bind_group && !opengl_pass->should_recreate_uniform_bind_group)
		return;

	if(opengl_pass->should_recreate_read_only_bind_group && opengl_pipeline->readonly_group_layout != PULSE_NULLPTR)
	{
		if(opengl_pass->read_only_bind_group != PULSE_NULLPTR)
			OpenGLReturnBindsGroupToPool(opengl_pass->read_only_bind_group->pool, opengl_pass->read_only_bind_group);
		opengl_pass->read_only_bind_group = OpenGLRequestBindsGroupFromPool(OpenGLGetAvailableBindsGroupPool(&opengl_device->binds_group_pool_manager), opengl_pipeline->readonly_group_layout);

		for(uint32_t i = 0; i < pass->current_pipeline->num_readonly_storage_images; i++)
		{
		}
		for(uint32_t i = 0; i < pass->current_pipeline->num_readonly_storage_buffers; i++)
			opengl_pass->read_only_bind_group->ReadOnly.storage_buffers[i] = pass->readonly_storage_buffers[i];
		opengl_pass->should_recreate_read_only_bind_group = false;
	}
	if(opengl_pass->should_recreate_write_bind_group && opengl_pipeline->readwrite_group_layout != PULSE_NULLPTR)
	{
		if(opengl_pass->read_write_bind_group != PULSE_NULLPTR)
			OpenGLReturnBindsGroupToPool(opengl_pass->read_write_bind_group->pool, opengl_pass->read_write_bind_group);
		opengl_pass->read_write_bind_group = OpenGLRequestBindsGroupFromPool(OpenGLGetAvailableBindsGroupPool(&opengl_device->binds_group_pool_manager), opengl_pipeline->readwrite_group_layout);

		for(uint32_t i = 0; i < pass->current_pipeline->num_readwrite_storage_images; i++)
		{
		}
		for(uint32_t i = 0; i < pass->current_pipeline->num_readwrite_storage_buffers; i++)
			opengl_pass->read_write_bind_group->ReadWrite.storage_buffers[i] = pass->readwrite_storage_buffers[i];
		opengl_pass->should_recreate_write_bind_group = false;
	}
	if(opengl_pass->should_recreate_uniform_bind_group && opengl_pipeline->uniform_group_layout != PULSE_NULLPTR)
	{
		if(opengl_pass->uniform_bind_group != PULSE_NULLPTR)
			OpenGLReturnBindsGroupToPool(opengl_pass->uniform_bind_group->pool, opengl_pass->uniform_bind_group);
		opengl_pass->uniform_bind_group = OpenGLRequestBindsGroupFromPool(OpenGLGetAvailableBindsGroupPool(&opengl_device->binds_group_pool_manager), opengl_pipeline->uniform_group_layout);

		for(uint32_t i = 0; i < pass->current_pipeline->num_uniform_buffers; i++)
			opengl_pass->uniform_bind_group->Uniform.buffers[i] = pass->uniform_buffers[i];
		opengl_pass->should_recreate_uniform_bind_group = false;
	}
}

void OpenGLBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	PULSE_UNUSED(pipeline);
	OpenGLComputePass* opengl_pass = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass, OpenGLComputePass*);
	opengl_pass->should_recreate_read_only_bind_group = true;
	opengl_pass->should_recreate_write_bind_group = true;
	opengl_pass->should_recreate_uniform_bind_group = true;
}

void OpenGLDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	OpenGLComputePass* opengl_pass = OPENGL_RETRIEVE_DRIVER_DATA_AS(pass, OpenGLComputePass*);
	OpenGLBindBindsGroup(pass);

	OpenGLCommand command = { 0 };
	command.type = OPENGL_COMMAND_DISPATCH;
	command.Dispatch.groupcount_x = groupcount_x;
	command.Dispatch.groupcount_y = groupcount_y;
	command.Dispatch.groupcount_z = groupcount_z;
	command.Dispatch.pipeline = pass->current_pipeline;

	if(opengl_pass->read_only_bind_group != PULSE_NULLPTR)
	{
		command.Dispatch.read_only_group = (OpenGLBindsGroup*)malloc(sizeof(OpenGLBindsGroup));
		PULSE_CHECK_ALLOCATION(command.Dispatch.read_only_group);
		memcpy(command.Dispatch.read_only_group, opengl_pass->read_only_bind_group, sizeof(OpenGLBindsGroup));
	}

	if(opengl_pass->read_write_bind_group != PULSE_NULLPTR)
	{
		command.Dispatch.read_write_group = (OpenGLBindsGroup*)malloc(sizeof(OpenGLBindsGroup));
		PULSE_CHECK_ALLOCATION(command.Dispatch.read_write_group);
		memcpy(command.Dispatch.read_write_group, opengl_pass->read_write_bind_group, sizeof(OpenGLBindsGroup));
	}

	if(opengl_pass->uniform_bind_group != PULSE_NULLPTR)
	{
		command.Dispatch.uniform_group = (OpenGLBindsGroup*)malloc(sizeof(OpenGLBindsGroup));
		PULSE_CHECK_ALLOCATION(command.Dispatch.uniform_group);
		memcpy(command.Dispatch.uniform_group, opengl_pass->uniform_bind_group, sizeof(OpenGLBindsGroup));
	}

	OpenGLQueueCommand(pass->cmd, command);
}
