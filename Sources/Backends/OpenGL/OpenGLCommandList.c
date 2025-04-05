// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLCommandList.h"
#include "OpenGLDevice.h"
#include "OpenGLBuffer.h"
#include "OpenGLFence.h"
#include "OpenGLComputePipeline.h"
#include "OpenGLComputePass.h"

static void OpenGLCommandCopyBufferToBuffer(PulseDevice device, OpenGLCommand* cmd)
{
	const PulseBufferRegion* src = cmd->CopyBufferToBuffer.src;
	const PulseBufferRegion* dst = cmd->CopyBufferToBuffer.dst;
	OpenGLBuffer* src_buffer = OPENGL_RETRIEVE_DRIVER_DATA_AS(src->buffer, OpenGLBuffer*);
	OpenGLBuffer* dst_buffer = OPENGL_RETRIEVE_DRIVER_DATA_AS(dst->buffer, OpenGLBuffer*);

	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	opengl_device->glBindBuffer(device, GL_COPY_READ_BUFFER, src_buffer->buffer);
	opengl_device->glBindBuffer(device, GL_COPY_WRITE_BUFFER, dst_buffer->buffer);
	opengl_device->glCopyBufferSubData(device, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, src->offset, dst->offset, src->size);

	free((void*)src);
	free((void*)dst);
}

static void OpenGLCommandDispatch(PulseDevice device, OpenGLCommand* cmd)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	OpenGLComputePipeline* opengl_pipeline = OPENGL_RETRIEVE_DRIVER_DATA_AS(cmd->Dispatch.pipeline, OpenGLComputePipeline*);
	opengl_device->glUseProgram(device, opengl_pipeline->program);
	opengl_device->glDispatchCompute(device, cmd->Dispatch.groupcount_x, cmd->Dispatch.groupcount_y, cmd->Dispatch.groupcount_z);
}

static void OpenGLCommandsRunner(PulseCommandList cmd)
{
	PULSE_CHECK_PTR(cmd);

	OpenGLCommandList* opengl_cmd = OPENGL_RETRIEVE_DRIVER_DATA_AS(cmd, OpenGLCommandList*);
	PULSE_CHECK_PTR(opengl_cmd);

	for(uint32_t i = 0; i < opengl_cmd->commands_count; i++)
	{
		OpenGLCommand* command = &opengl_cmd->commands[i];
		switch(command->type)
		{
			case OPENGL_COMMAND_COPY_BUFFER_TO_BUFFER: OpenGLCommandCopyBufferToBuffer(cmd->device, command); break;
			case OPENGL_COMMAND_COPY_BUFFER_TO_IMAGE: break;
			case OPENGL_COMMAND_COPY_IMAGE_TO_BUFFER: break;
			case OPENGL_COMMAND_DISPATCH: OpenGLCommandDispatch(cmd->device, command); break;
			case OPENGL_COMMAND_DISPATCH_INDIRECT: break;

			default: break;
		}
	}
	cmd->state = PULSE_COMMAND_LIST_STATE_READY;
}

PulseCommandList OpenGLRequestCommandList(PulseDevice device, PulseCommandListUsage usage)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);

	PULSE_UNUSED(usage);

	PulseCommandList cmd = (PulseCommandList)calloc(1, sizeof(PulseCommandListHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(cmd, PULSE_NULL_HANDLE);

	OpenGLCommandList* opengl_cmd = (OpenGLCommandList*)calloc(1, sizeof(OpenGLCommandList));
	PULSE_CHECK_ALLOCATION_RETVAL(opengl_cmd, PULSE_NULL_HANDLE);

	cmd->usage = usage;
	cmd->device = device;
	cmd->driver_data = opengl_cmd;
	cmd->thread_id = PulseGetThreadID();

	cmd->pass = OpenGLCreateComputePass(device, cmd);
	cmd->state = PULSE_COMMAND_LIST_STATE_RECORDING;
	cmd->is_available = false;

	return cmd;
}

void OpenGLQueueCommand(PulseCommandList cmd, OpenGLCommand command)
{
	OpenGLCommandList* opengl_cmd = OPENGL_RETRIEVE_DRIVER_DATA_AS(cmd, OpenGLCommandList*);
	PULSE_EXPAND_ARRAY_IF_NEEDED(opengl_cmd->commands, OpenGLCommand, opengl_cmd->commands_count, opengl_cmd->commands_capacity, 8);
	opengl_cmd->commands[opengl_cmd->commands_count] = command;
	opengl_cmd->commands_count++;
}

bool OpenGLSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence)
{
	PULSE_UNUSED(device);
	cmd->state = PULSE_COMMAND_LIST_STATE_SENT;
	if(fence != PULSE_NULL_HANDLE)
		fence->cmd = cmd;
	OpenGLCommandsRunner(cmd);
	return true;
}

void OpenGLReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	OpenGLDestroyComputePass(device, cmd->pass);
	free(cmd->driver_data);
	free(cmd);
}
