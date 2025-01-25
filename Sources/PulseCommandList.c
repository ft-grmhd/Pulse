// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>
#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseCommandList PulseRequestCommandList(PulseDevice device, PulseCommandListUsage usage)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	return device->PFN_RequestCommandList(device, usage);
}

PULSE_API bool PulseSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	PULSE_CHECK_HANDLE_RETVAL(cmd, false);

	if(cmd->state != PULSE_COMMAND_LIST_STATE_RECORDING)
	{
		switch(cmd->state)
		{
			case PULSE_COMMAND_LIST_STATE_INVALID:
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
					PulseLogError(device->backend, "command list is in invalid state");
			return false;

			case PULSE_COMMAND_LIST_STATE_READY:
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
					PulseLogError(device->backend, "command list is empty");
			return false;

			case PULSE_COMMAND_LIST_STATE_SENT:
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
					PulseLogWarning(device->backend, "command list has already been submitted");
			return false;

			default: break;
		}
	}

	if(cmd->pass->is_recording)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "submitting command list with a recording compute pass, stopping record");
		PulseEndComputePass(cmd->pass);
	}

	memset(cmd->pass->compute_pipelines_bound, 0, sizeof(PulseComputePipeline) * cmd->pass->compute_pipelines_bound_size);
	cmd->pass->compute_pipelines_bound_size = 0;

	return device->PFN_SubmitCommandList(device, cmd, fence);
}

PULSE_API void PulseReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	PULSE_CHECK_HANDLE(device);

	if(cmd == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "command list is NULL, this may be a bug in your application");
		return;
	}

	switch(cmd->state)
	{
		case PULSE_COMMAND_LIST_STATE_SENT:
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
				PulseLogWarning(device->backend, "command list is in pending state");
		break;

		default: break;
	}
	return device->PFN_ReleaseCommandList(device, cmd);
}

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
