// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGLBuffer.h"
#include "OpenGLDevice.h"
#include "OpenGLCommandList.h"

PulseBuffer OpenGLCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	PulseBufferHandler* buffer = (PulseBufferHandler*)calloc(1, sizeof(PulseBufferHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(buffer, PULSE_NULL_HANDLE);

	OpenGLBuffer* opengl_buffer = (OpenGLBuffer*)calloc(1, sizeof(OpenGLBuffer));
	PULSE_CHECK_ALLOCATION_RETVAL(opengl_buffer, PULSE_NULL_HANDLE);

	buffer->device = device;
	buffer->driver_data = opengl_buffer;
	buffer->size = create_infos->size;
	buffer->usage = create_infos->usage;

	void* data = calloc(1, create_infos->size);
	PULSE_CHECK_ALLOCATION_RETVAL(data, PULSE_NULL_HANDLE);

	opengl_device->glGenBuffers(device, 1, &opengl_buffer->buffer);
	opengl_device->glBindBuffer(device, GL_SHADER_STORAGE_BUFFER, opengl_buffer->buffer);
	opengl_device->glBufferData(device, GL_SHADER_STORAGE_BUFFER, create_infos->size, data, GL_DYNAMIC_COPY);

	free(data);

	return buffer;
}

bool OpenGLMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data)
{
	OpenGLBuffer* opengl_buffer = OPENGL_RETRIEVE_DRIVER_DATA_AS(buffer, OpenGLBuffer*);
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(buffer->device, OpenGLDevice*);
	opengl_device->glBindBuffer(buffer->device, GL_SHADER_STORAGE_BUFFER, opengl_buffer->buffer);
	*data = opengl_device->glMapBufferRange(buffer->device, GL_SHADER_STORAGE_BUFFER, 0, buffer->size, mode == PULSE_MAP_READ ? GL_MAP_READ_BIT : GL_MAP_WRITE_BIT);
	return *data != PULSE_NULLPTR;
}

void OpenGLUnmapBuffer(PulseBuffer buffer)
{
	OpenGLBuffer* opengl_buffer = OPENGL_RETRIEVE_DRIVER_DATA_AS(buffer, OpenGLBuffer*);
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(buffer->device, OpenGLDevice*);
	opengl_device->glBindBuffer(buffer->device, GL_SHADER_STORAGE_BUFFER, opengl_buffer->buffer);
	opengl_device->glUnmapBuffer(buffer->device, GL_SHADER_STORAGE_BUFFER);
}

bool OpenGLCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
	OpenGLCommand command = { 0 };
	command.type = OPENGL_COMMAND_COPY_BUFFER_TO_BUFFER;
	command.CopyBufferToBuffer.src = (PulseBufferRegion*)malloc(sizeof(PulseBufferRegion));
	command.CopyBufferToBuffer.dst = (PulseBufferRegion*)malloc(sizeof(PulseBufferRegion));
	memcpy((void*)command.CopyBufferToBuffer.src, src, sizeof(PulseBufferRegion));
	memcpy((void*)command.CopyBufferToBuffer.dst, dst, sizeof(PulseBufferRegion));
	OpenGLQueueCommand(cmd, command);
	return true;
}

bool OpenGLCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
	OpenGLCommand command = { 0 };
	command.type = OPENGL_COMMAND_COPY_BUFFER_TO_IMAGE;
	command.CopyBufferToImage.src = (PulseBufferRegion*)malloc(sizeof(PulseBufferRegion));
	command.CopyBufferToImage.dst = (PulseImageRegion*)malloc(sizeof(PulseImageRegion));
	memcpy((void*)command.CopyBufferToImage.src, src, sizeof(PulseBufferRegion));
	memcpy((void*)command.CopyBufferToImage.dst, dst, sizeof(PulseImageRegion));
	OpenGLQueueCommand(cmd, command);
	return true;
}

void OpenGLDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	OpenGLBuffer* opengl_buffer = OPENGL_RETRIEVE_DRIVER_DATA_AS(buffer, OpenGLBuffer*);
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	opengl_device->glDeleteBuffers(device, 1, &opengl_buffer->buffer);
	free(opengl_buffer);
	free(buffer);
}
