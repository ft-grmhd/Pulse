// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftComputePass.h"
#include "SoftCommandList.h"

PulseComputePass SoftCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PULSE_UNUSED(device);
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	pass->cmd = cmd;

	return pass;
}

void SoftDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
	PULSE_UNUSED(device);
	free(pass->driver_data);
	free(pass);
}

PulseComputePass SoftBeginComputePass(PulseCommandList cmd)
{
	return cmd->pass;
}

void SoftEndComputePass(PulseComputePass pass)
{
	PULSE_UNUSED(pass);
}

void SoftBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
	PulseBufferUsageFlags usage = buffers[0]->usage;
	bool is_readwrite = (usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) != 0;
	PulseBuffer* array = is_readwrite ? pass->readwrite_storage_buffers : pass->readonly_storage_buffers;

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
	}
}

void SoftBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void SoftBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
}

void SoftBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	PULSE_UNUSED(pass);
	PULSE_UNUSED(pipeline);
}

void SoftDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	SoftCommand command = { 0 };
	command.type = SOFT_COMMAND_DISPATCH;
	command.Dispatch.groupcount_x = groupcount_x;
	command.Dispatch.groupcount_y = groupcount_y;
	command.Dispatch.groupcount_z = groupcount_z;
	command.Dispatch.pipeline = pass->current_pipeline;
	mtx_init(&command.Dispatch.dispatch_mutex, mtx_plain);
	SoftQueueCommand(pass->cmd, command);
}
