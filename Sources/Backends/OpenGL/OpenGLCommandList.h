// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_COMMAND_LIST_H_
#define PULSE_OPENGL_COMMAND_LIST_H_

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLBuffer.h"
#include "OpenGLBindsGroup.h"

typedef struct OpenGLCommand
{
	OpenGLCommandType type;
	union
	{
		struct
		{
			const PulseImageRegion* src;
			const PulseImageRegion* dst;
		} BlitImages;

		struct
		{
			const PulseBufferRegion* src;
			const PulseBufferRegion* dst;
		} CopyBufferToBuffer;

		struct
		{
			const PulseBufferRegion* src;
			const PulseImageRegion* dst;
		} CopyBufferToImage;

		struct
		{
			const PulseImageRegion* src;
			const PulseBufferRegion* dst;
		} CopyImageToBuffer;

		struct
		{
			OpenGLBindsGroup* read_only_group;
			OpenGLBindsGroup* read_write_group;
			OpenGLBindsGroup* uniform_group;
			PulseComputePipeline pipeline;
			uint32_t groupcount_x;
			uint32_t groupcount_y;
			uint32_t groupcount_z;
		} Dispatch;

		struct
		{
			OpenGLBindsGroup* read_only_group;
			OpenGLBindsGroup* read_write_group;
			OpenGLBindsGroup* uniform_group;
			PulseComputePipeline pipeline;
			PulseBuffer buffer;
			uint32_t offset;
		} DispatchIndirect;
	};
} OpenGLCommand;

typedef struct OpenGLCommandList
{
	OpenGLCommand* commands;
	uint32_t commands_count;
	uint32_t commands_capacity;
} OpenGLCommandList;

PulseCommandList OpenGLRequestCommandList(PulseDevice device, PulseCommandListUsage usage);
void OpenGLQueueCommand(PulseCommandList cmd, OpenGLCommand command);
bool OpenGLSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
void OpenGLReleaseCommandList(PulseDevice device, PulseCommandList cmd);

#endif // PULSE_OPENGL_COMMAND_LIST_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
