// Copyright (C) 2025 kanel
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
