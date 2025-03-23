// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLCommandList.h"

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
			case OPENGL_COMMAND_COPY_BUFFER_TO_BUFFER: OpenGLCommandCopyBufferToBuffer(command); break;
			case OPENGL_COMMAND_COPY_BUFFER_TO_IMAGE: break;
			case OPENGL_COMMAND_COPY_IMAGE_TO_BUFFER: break;
			case OPENGL_COMMAND_DISPATCH: OpenGLCommandDispatch(command); break;
			case OPENGL_COMMAND_DISPATCH_INDIRECT: break;

			default: break;
		}
	}

	if(opengl_cmd->fence != PULSE_NULL_HANDLE)
	{
		OpenGLFence* fence = OPENGL_RETRIEVE_DRIVER_DATA_AS(opengl_cmd->fence, OpenGLFence*);
		atomic_store(&fence->signal, true);
	}
	cmd->state = PULSE_COMMAND_LIST_STATE_READY;
}

PulseCommandList OpenGLRequestCommandList(PulseDevice device, PulseCommandListUsage usage)
{
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
}

void OpenGLReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
}
