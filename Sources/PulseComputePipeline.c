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
	return device->PFN_CreateComputePipeline(device, info);
}

PULSE_API void PulseDispatchComputePipeline(PulseComputePipeline pipeline, PulseCommandList cmd, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	PULSE_CHECK_HANDLE(pipeline);
	PULSE_CHECK_HANDLE(cmd);

	if(cmd->state != PULSE_COMMAND_LIST_STATE_RECORDING)
	{
		switch(cmd->state)
		{
			case PULSE_COMMAND_LIST_STATE_INVALID:
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(cmd->device->backend))
					PulseLogError(cmd->device->backend, "command list is in invalid state");
			return;

			case PULSE_COMMAND_LIST_STATE_READY:
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(cmd->device->backend))
					PulseLogError(cmd->device->backend, "command list is not recording");
			return;

			case PULSE_COMMAND_LIST_STATE_SENT:
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(cmd->device->backend))
					PulseLogWarning(cmd->device->backend, "command list has already been submitted");
			return;

			default: break;
		}
	}

	cmd->device->PFN_DispatchComputePipeline(pipeline, cmd, groupcount_x, groupcount_y, groupcount_z);
	pipeline->cmd = cmd;

	if(cmd->compute_pipelines_bound_size == cmd->compute_pipelines_bound_capacity)
	{
		cmd->compute_pipelines_bound_capacity += 5;
		cmd->compute_pipelines_bound = (PulseComputePipeline*)realloc(cmd->compute_pipelines_bound, cmd->compute_pipelines_bound_capacity * sizeof(PulseComputePipeline));
		PULSE_CHECK_ALLOCATION(cmd->compute_pipelines_bound);
	}
	cmd->compute_pipelines_bound[cmd->compute_pipelines_bound_size] = pipeline;
	cmd->compute_pipelines_bound_size++;
}

PULSE_API void PulseDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	PULSE_CHECK_HANDLE(device);
	device->PFN_DestroyComputePipeline(device, pipeline);
}
