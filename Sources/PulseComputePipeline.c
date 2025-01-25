// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "PulseDefs.h"
#include "PulseInternal.h"


PULSE_API PulseComputePipeline PulseCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	if(info == PULSE_NULLPTR && PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
		PulseLogError(device->backend, "null infos pointer");
	PULSE_CHECK_PTR_RETVAL(info, PULSE_NULL_HANDLE);
	PulseComputePipeline pipeline = device->PFN_CreateComputePipeline(device, info);
	if(pipeline == PULSE_NULL_HANDLE)
		return PULSE_NULL_HANDLE;
	pipeline->num_readonly_storage_images = info->num_readonly_storage_images;
	pipeline->num_readonly_storage_buffers = info->num_readonly_storage_buffers;
	pipeline->num_readwrite_storage_images = info->num_readwrite_storage_buffers;
	pipeline->num_readwrite_storage_buffers = info->num_readwrite_storage_buffers;
	pipeline->num_uniform_buffers = info->num_uniform_buffers;
	return pipeline;
}

PULSE_API void PulseBindStorageBuffers(PulseComputePass pass, uint32_t starting_slot, PulseBuffer* const* buffers, uint32_t num_buffers)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindStorageBuffers(pass, starting_slot, buffers, num_buffers);
}

PULSE_API void PulseBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindUniformData(pass, slot, data, data_size);
}

PULSE_API void PulseBindStorageImages(PulseComputePass pass, uint32_t starting_slot, PulseImage* const* images, uint32_t num_images)
{
	PULSE_CHECK_HANDLE(pass);

	PULSE_CHECK_COMMAND_LIST_STATE(pass->cmd);

	pass->cmd->device->PFN_BindStorageImages(pass, starting_slot, images, num_images);
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

	pass->cmd->device->PFN_DispatchComputations(pass, groupcount_x, groupcount_y, groupcount_z);
}

PULSE_API void PulseDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	PULSE_CHECK_HANDLE(device);
	device->PFN_DestroyComputePipeline(device, pipeline);
}
