// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>
#include <stdatomic.h>
#include <tinycthread.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftFence.h"
#include "SoftDevice.h"
#include "SoftCommandList.h"
#include "SoftComputePass.h"
#include "SoftBuffer.h"

static void SoftCommandCopyBufferToBuffer(SoftCommand* cmd)
{
	const PulseBufferRegion* src = cmd->CopyBufferToBuffer.src;
	const PulseBufferRegion* dst = cmd->CopyBufferToBuffer.dst;
	SoftBuffer* src_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(src->buffer, SoftBuffer*);
	SoftBuffer* dst_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(dst->buffer, SoftBuffer*);
	memcpy(dst_buffer->buffer + dst->offset, src_buffer->buffer + src->offset, (src->size < dst->size ? src->size : dst->size));
	//free((void*)src);
	//free((void*)dst);
}

static int SoftCommandsRunner(void* arg)
{
	PulseCommandList cmd = (PulseCommandList)arg;
	PULSE_CHECK_PTR_RETVAL(cmd, 1);

	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	PULSE_CHECK_PTR_RETVAL(soft_cmd, 1);

	for(uint32_t i = 0; i < soft_cmd->commands_count; i++)
	{
		SoftCommand* command = &soft_cmd->commands[i];
		switch(command->type)
		{
			case SOFT_COMMAND_BIND_COMPUTE_PIPELINE: break;
			case SOFT_COMMAND_BIND_STORAGE_BUFFERS: break;
			case SOFT_COMMAND_BIND_STORAGE_IMAGES: break;
			case SOFT_COMMAND_BIND_UNIFORM_BUFFERS: break;
			case SOFT_COMMAND_BLIT_IMAGES: break;
			case SOFT_COMMAND_COPY_BUFFER_TO_BUFFER: SoftCommandCopyBufferToBuffer(command); break;
			case SOFT_COMMAND_COPY_BUFFER_TO_IMAGE: break;
			case SOFT_COMMAND_COPY_IMAGE_TO_BUFFER: break;
			case SOFT_COMMAND_DISPATCH: break;
			case SOFT_COMMAND_DISPATCH_INDIRECT: break;

			default: break;
		}
	}

	if(soft_cmd->fence != PULSE_NULL_HANDLE)
	{
		SoftFence* fence = SOFT_RETRIEVE_DRIVER_DATA_AS(soft_cmd->fence, SoftFence*);
		atomic_store(&fence->signal, true);
	}
	cmd->state = PULSE_COMMAND_LIST_STATE_READY;
	return 0;
}

PulseCommandList SoftRequestCommandList(PulseDevice device, PulseCommandListUsage usage)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);

	PULSE_UNUSED(usage);

	PulseCommandList cmd = (PulseCommandList)calloc(1, sizeof(PulseCommandListHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(cmd, PULSE_NULL_HANDLE);

	SoftCommandList* soft_cmd = (SoftCommandList*)calloc(1, sizeof(SoftCommandList));
	PULSE_CHECK_ALLOCATION_RETVAL(soft_cmd, PULSE_NULL_HANDLE);

	cmd->usage = usage;
	cmd->device = device;
	cmd->driver_data = soft_cmd;
	cmd->thread_id = PulseGetThreadID();

	cmd->pass = SoftCreateComputePass(device, cmd);
	cmd->state = PULSE_COMMAND_LIST_STATE_RECORDING;
	cmd->is_available = false;

	return cmd;
}

void SoftQueueCommand(PulseCommandList cmd, SoftCommand command)
{
	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	PULSE_EXPAND_ARRAY_IF_NEEDED(soft_cmd->commands, SoftCommand, soft_cmd->commands_count, soft_cmd->commands_capacity, 8);
	soft_cmd->commands[soft_cmd->commands_count] = command;
	soft_cmd->commands_count++;
}

bool SoftSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence)
{
	PULSE_UNUSED(device);
	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	cmd->state = PULSE_COMMAND_LIST_STATE_SENT;
	soft_cmd->fence = fence;
	return thrd_create(&soft_cmd->thread, SoftCommandsRunner, cmd) == thrd_success;
}

#include <stdio.h>
void SoftReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	printf("%p, %p, %p\n", cmd, soft_cmd, cmd->pass);
	SoftDestroyComputePass(device, cmd->pass);
	free(soft_cmd);
	free(cmd);
}
