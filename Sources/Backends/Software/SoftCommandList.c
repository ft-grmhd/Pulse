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
	SoftCommand* cmd = (SoftCommand*)arg;
	SoftComputePipeline* soft_pipeline = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd->Dispatch.pipeline, SoftComputePipeline*);

	mtx_lock(&cmd->Dispatch.dispatch_mutex);
		spvm_state_t state = spvm_state_create(soft_pipeline->program);
	mtx_unlock(&cmd->Dispatch.dispatch_mutex);

	spvm_ext_opcode_func* glsl_ext_data = spvm_build_glsl450_ext();
	spvm_result_t glsl_std_450 = spvm_state_get_result(state, "GLSL.std.450");
	if(glsl_std_450)
		glsl_std_450->extension = glsl_ext_data;
	spvm_word main = spvm_state_get_result_location(state, (spvm_string)soft_pipeline->entry_point);

	spvm_word mem_count = 0;
	spvm_member_t local_invocation_id = spvm_state_get_builtin(state, SpvBuiltInLocalInvocationId, &mem_count);

	spvm_state_prepare(state, main);
	spvm_state_call_function(state);
	spvm_state_delete(state);
	atomic_fetch_sub(&cmd->cmd_list->commands_running, 1);
	return 0;
}

static void SoftCommandDispatch(SoftCommand* cmd)
{
	SoftComputePipeline* soft_pipeline = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd->Dispatch.pipeline, SoftComputePipeline*);
	uint32_t local_size = soft_pipeline->program->local_size_x * soft_pipeline->program->local_size_y * soft_pipeline->program->local_size_z;
	uint32_t invocations_count = cmd->Dispatch.groupcount_x * cmd->Dispatch.groupcount_y * cmd->Dispatch.groupcount_z * local_size;
	thrd_t* invocations = (thrd_t*)malloc(invocations_count * sizeof(thrd_t));
	PULSE_CHECK_PTR(invocations);

	uint32_t invocation_index = 0;
	for(uint32_t z = 0; z < cmd->Dispatch.groupcount_z; z++)
	{
		for(uint32_t y = 0; y < cmd->Dispatch.groupcount_y; y++)
		{
			for(uint32_t x = 0; x < cmd->Dispatch.groupcount_x; x++)
			{
				for(uint32_t i = 0; i < local_size; i++)
				{
					atomic_fetch_add(&cmd->cmd_list->commands_running, 1);
					thrd_create(&invocations[invocation_index], SoftCommandDispatchCore, cmd);
					invocation_index++;
				}
			}
		}
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

	atomic_fetch_sub(&soft_cmd->commands_running, 1); // Remove fence safety

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
	atomic_store(&soft_cmd->commands_running, 0);

	return cmd;
}

void SoftQueueCommand(PulseCommandList cmd, SoftCommand command)
{
	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	command.cmd_list = soft_cmd;
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
	atomic_fetch_add(&soft_cmd->commands_running, 1); // Fence safety to avoid fence being signaled before first command being sumitted
	return thrd_create(&soft_cmd->thread, SoftCommandsRunner, cmd) == thrd_success;
}

void SoftReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	SoftCommandList* soft_cmd = SOFT_RETRIEVE_DRIVER_DATA_AS(cmd, SoftCommandList*);
	SoftDestroyComputePass(device, cmd->pass);

	for(uint32_t i = 0; i < soft_cmd->commands_count; i++)
	{
		SoftCommand* command = &soft_cmd->commands[i];
		switch(command->type)
		{
			// Lock/Unlock to make sure the mutex is not in use
			case SOFT_COMMAND_DISPATCH: mtx_lock(&command->Dispatch.dispatch_mutex); mtx_unlock(&command->Dispatch.dispatch_mutex); break;
			case SOFT_COMMAND_DISPATCH_INDIRECT: mtx_lock(&command->DispatchIndirect.dispatch_mutex); mtx_unlock(&command->DispatchIndirect.dispatch_mutex); break;

			default: break;
		}
	}
	free(soft_cmd->commands);
	free(soft_cmd);
	free(cmd);
}
