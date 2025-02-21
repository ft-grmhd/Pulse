// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>
#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseComputePass PulseBeginComputePass(PulseCommandList cmd)
{
	PULSE_CHECK_HANDLE_RETVAL(cmd, PULSE_NULL_HANDLE);
	PULSE_CHECK_HANDLE_RETVAL(cmd->device, PULSE_NULL_HANDLE);

	PULSE_CHECK_COMMAND_LIST_STATE_RETVAL(cmd, PULSE_NULL_HANDLE);
	PulseComputePass pass = cmd->device->PFN_BeginComputePass(cmd);
	if(pass->is_recording == true)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(cmd->device->backend))
			PulseLogWarning(cmd->device->backend, "a compute pass is already recording in this command buffer, please call PulseEndComputePass before beginning a new one");
		return PULSE_NULL_HANDLE;
	}
	pass->is_recording = true;
	return pass;
}

PULSE_API void PulseBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindStorageBuffers(pass, buffers, num_buffers);
}

PULSE_API void PulseBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindUniformData(pass, slot, data, data_size);
}

PULSE_API void PulseBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindStorageImages(pass, images, num_images);
}

PULSE_API void PulseBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	PULSE_CHECK_HANDLE(pass);
	PULSE_CHECK_HANDLE(pipeline);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindComputePipeline(pass, pipeline);

	pass->current_pipeline = pipeline;

	PULSE_EXPAND_ARRAY_IF_NEEDED(pass->compute_pipelines_bound, PulseComputePipeline, pass->compute_pipelines_bound_size, pass->compute_pipelines_bound_capacity, 2);
	pass->compute_pipelines_bound[pass->compute_pipelines_bound_size] = pipeline;
	pass->compute_pipelines_bound_size++;
}

PULSE_API void PulseDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	if(pass->current_pipeline == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
			PulseLogWarning(pass->cmd->device->backend, "cannot dispatch computations, no pipeline bound");
		PulseSetInternalError(PULSE_ERROR_INVALID_HANDLE);
	}

	pass->cmd->device->PFN_DispatchComputations(pass, groupcount_x, groupcount_y, groupcount_z);
}

PULSE_API void PulseEndComputePass(PulseComputePass pass)
{
	if(pass == PULSE_NULL_HANDLE)
	{
		PULSE_CHECK_HANDLE(pass->cmd);
		PULSE_CHECK_HANDLE(pass->cmd->device);
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
			PulseLogWarning(pass->cmd->device->backend, "command list is NULL, this may be a bug in your application");
		return;
	}
	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	memset(pass->readonly_images, 0, PULSE_MAX_READ_TEXTURES_BOUND * sizeof(PulseImage));
	memset(pass->readwrite_images, 0, PULSE_MAX_WRITE_TEXTURES_BOUND * sizeof(PulseImage));
	memset(pass->readonly_storage_buffers, 0, PULSE_MAX_READ_BUFFERS_BOUND * sizeof(PulseBuffer));
	memset(pass->readwrite_storage_buffers, 0, PULSE_MAX_WRITE_BUFFERS_BOUND * sizeof(PulseBuffer));

	pass->current_pipeline = PULSE_NULL_HANDLE;

	pass->is_recording = false;
}
