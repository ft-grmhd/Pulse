// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftBuffer.h"
#include "SoftCommandList.h"

PulseBuffer SoftCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
	PulseBuffer buffer = (PulseBuffer)calloc(1, sizeof(PulseBufferHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(buffer, PULSE_NULL_HANDLE);

	SoftBuffer* soft_buffer = (SoftBuffer*)calloc(1, sizeof(SoftBuffer) + _Alignof(SoftBuffer) + create_infos->size);
	PULSE_CHECK_ALLOCATION_RETVAL(soft_buffer, PULSE_NULL_HANDLE);

	buffer->device = device;
	buffer->driver_data = soft_buffer;
	buffer->size = create_infos->size;
	buffer->usage = create_infos->usage;

	soft_buffer->buffer = soft_buffer + sizeof(SoftBuffer) + _Alignof(SoftBuffer);

	return buffer;
}

bool SoftMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data)
{
	SoftBuffer* soft_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(buffer, SoftBuffer*);

	if(mode == PULSE_MAP_WRITE)
	{
		soft_buffer->map = calloc(1, buffer->size);
		PULSE_CHECK_ALLOCATION_RETVAL(soft_buffer->map, false);
	}
	else
		soft_buffer->map = soft_buffer->buffer;
	if(soft_buffer->map == PULSE_NULLPTR)
		return false;
	soft_buffer->current_map_mode = mode;
	*data = soft_buffer->map;
	return true;
}

void SoftUnmapBuffer(PulseBuffer buffer)
{
	SoftBuffer* soft_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(buffer, SoftBuffer*);
	if(soft_buffer->current_map_mode == PULSE_MAP_WRITE)
	{
		memcpy(soft_buffer->buffer, soft_buffer->map, buffer->size);
		free(soft_buffer->map);
	}
	soft_buffer->map = PULSE_NULLPTR;
}

bool SoftCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
	SoftCommand command = { 0 };
	command.type = SOFT_COMMAND_COPY_BUFFER_TO_BUFFER;
	command.CopyBufferToBuffer.src = (PulseBufferRegion*)malloc(sizeof(PulseBufferRegion));
	command.CopyBufferToBuffer.dst = (PulseBufferRegion*)malloc(sizeof(PulseBufferRegion));
	memcpy((void*)command.CopyBufferToBuffer.src, src, sizeof(PulseBufferRegion));
	memcpy((void*)command.CopyBufferToBuffer.dst, dst, sizeof(PulseBufferRegion));
	SoftQueueCommand(cmd, command);
	return true;
}

bool SoftCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
	SoftCommand command = { 0 };
	command.type = SOFT_COMMAND_COPY_BUFFER_TO_IMAGE;
	command.CopyBufferToImage.src = (PulseBufferRegion*)malloc(sizeof(PulseBufferRegion));
	command.CopyBufferToImage.dst = (PulseImageRegion*)malloc(sizeof(PulseImageRegion));
	memcpy((void*)command.CopyBufferToImage.src, src, sizeof(PulseBufferRegion));
	memcpy((void*)command.CopyBufferToImage.dst, dst, sizeof(PulseImageRegion));
	SoftQueueCommand(cmd, command);
	return true;
}

void SoftDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	PULSE_UNUSED(device);
	SoftBuffer* soft_buffer = SOFT_RETRIEVE_DRIVER_DATA_AS(buffer, SoftBuffer*);
	free(soft_buffer);
	free(buffer);
}
