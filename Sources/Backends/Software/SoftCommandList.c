// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>
#include <stdatomic.h>
#include <tinycthread.h>
#include <spvm/ext/GLSL450.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftFence.h"
#include "SoftDevice.h"
#include "SoftCommandList.h"
#include "SoftComputePass.h"
#include "SoftComputePipeline.h"
#include "SoftBuffer.h"

#include <stdio.h>

static void SoftCommandCopyBufferToBuffer(SoftCommand* cmd)
{
	const PulseBufferRegion* src = cmd->CopyBufferToBuffer.src;
	const PulseBufferRegion* dst = cmd->CopyBufferToBuffer.dst;
	SoftBuffer* src_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(src->buffer, SoftBuffer*);
	SoftBuffer* dst_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(dst->buffer, SoftBuffer*);
	memcpy(dst_buffer->buffer + dst->offset, src_buffer->buffer + src->offset, (src->size < dst->size ? src->size : dst->size));
	free((void*)src);
	free((void*)dst);
}

static int SoftCommandDispatchCore(void* arg)
{
	SoftComputePipeline* soft_pipeline = (SoftComputePipeline*)arg;
	spvm_state_t state = spvm_state_create(soft_pipeline->program);
	spvm_ext_opcode_func* glsl_ext_data = spvm_build_glsl450_ext();
	spvm_result_t glsl_std_450 = spvm_state_get_result(state, "GLSL.std.450");
	if(glsl_std_450)
		glsl_std_450->extension = glsl_ext_data;
	spvm_word main = spvm_state_get_result_location(state, (spvm_string)soft_pipeline->entry_point);
	spvm_state_prepare(state, main);
	spvm_state_call_function(state);
	spvm_state_delete(state);
	return 0;
}

static void SoftCommandDispatch(SoftCommand* cmd)
{
	SoftComputePipeline* soft_pipeline = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd->Dispatch.pipeline, SoftComputePipeline*);
	uint32_t local_size = soft_pipeline->program->local_size_x * soft_pipeline->program->local_size_y * soft_pipeline->program->local_size_z;
	uint32_t invocations_count = cmd->Dispatch.groupcount_x * cmd->Dispatch.groupcount_y * cmd->Dispatch.groupcount_z * local_size;
	thrd_t* invocations = (thrd_t*)malloc(invocations_count * sizeof(thrd_t));
	PULSE_CHECK_PTR(invocations);

	printf("test2 %d %d\n", invocations_count, local_size);
	uint32_t invocation_index = 0;
	for(uint32_t z = 0; z < cmd->Dispatch.groupcount_z; z++)
	{
		for(uint32_t y = 0; y < cmd->Dispatch.groupcount_y; y++)
		{
			for(uint32_t x = 0; x < cmd->Dispatch.groupcount_x; x++)
			{
				for(uint32_t i = 0; i < local_size; i++)
				{
					printf("\r%d", invocation_index);
					thrd_create(&invocations[invocation_index], SoftCommandDispatchCore, soft_pipeline);
					invocation_index++;
				}
			}
		}
	}
	printf("\ntest %d %d %d\n", invocation_index, invocations_count, local_size);
	for(uint32_t i = 0; i < invocations_count; i++)
	{
		printf("test %d\n", i);
		int res;
		thrd_join(invocations[i], &res);
		PULSE_UNUSED(res);
	}
	free(invocations);
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
			case SOFT_COMMAND_COPY_BUFFER_TO_BUFFER: SoftCommandCopyBufferToBuffer(command); break;
			case SOFT_COMMAND_COPY_BUFFER_TO_IMAGE: break;
			case SOFT_COMMAND_COPY_IMAGE_TO_BUFFER: break;
			case SOFT_COMMAND_DISPATCH: SoftCommandDispatch(command); break;
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
	if(fence != PULSE_NULL_HANDLE)
	{
		SoftFence* soft_fence = SOFT_RETRIEVE_DRIVER_DATA_AS(fence, SoftFence*);
		soft_cmd->fence = fence;
		fence->cmd = cmd;
		atomic_store(&soft_fence->signal, false);
	}
	return thrd_create(&soft_cmd->thread, SoftCommandsRunner, cmd) == thrd_success;
}

void SoftReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	SoftDestroyComputePass(device, cmd->pass);
	free(soft_cmd);
	free(cmd);
}
