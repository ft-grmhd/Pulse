// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLComputePass.h"

PulseComputePass OpenGLCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PULSE_UNUSED(device);
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	OpenGLComputePass* opengl_pass = (OpenGLComputePass*)calloc(1, sizeof(OpenGLComputePass));
	PULSE_CHECK_ALLOCATION_RETVAL(opengl_pass, PULSE_NULL_HANDLE);

	pass->cmd = cmd;
	pass->driver_data = opengl_pass;

	return pass;
}

void OpenGLDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
	PULSE_UNUSED(device);
	free(pass->driver_data);
	free(pass);
}

PulseComputePass OpenGLBeginComputePass(PulseCommandList cmd)
{
	return cmd->pass;
}

void OpenGLEndComputePass(PulseComputePass pass)
{
	PULSE_UNUSED(pass);
}

void OpenGLBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
}

void OpenGLBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void OpenGLBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
}

void OpenGLBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	PULSE_UNUSED(pass);
	PULSE_UNUSED(pipeline);
}

void OpenGLDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	OpenGLCommand command = { 0 };
	command.type = OPENGL_COMMAND_DISPATCH;
	command.Dispatch.groupcount_x = groupcount_x;
	command.Dispatch.groupcount_y = groupcount_y;
	command.Dispatch.groupcount_z = groupcount_z;
	command.Dispatch.pipeline = pass->current_pipeline;
	OpenGLQueueCommand(pass->cmd, command);
}
