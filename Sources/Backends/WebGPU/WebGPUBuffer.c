// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdatomic.h>
#include <time.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "webgpu.h"
#include "WebGPUBuffer.h"

PulseBuffer WebGPUCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);

	PulseBufferHandler* buffer = (PulseBufferHandler*)calloc(1, sizeof(PulseBufferHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(buffer, PULSE_NULL_HANDLE);

	WebGPUBuffer* webgpu_buffer = (WebGPUBuffer*)calloc(1, sizeof(WebGPUBuffer));
	PULSE_CHECK_ALLOCATION_RETVAL(webgpu_buffer, PULSE_NULL_HANDLE);

	buffer->device = device;
	buffer->driver_data = webgpu_buffer;
	buffer->size = create_infos->size;
	buffer->usage = create_infos->usage;

	bool is_storage = false;

	WGPUBufferDescriptor descriptor = { 0 };
	descriptor.mappedAtCreation = false;
	descriptor.size = buffer->size;
	if(buffer->usage & PULSE_BUFFER_USAGE_STORAGE_READ || buffer->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE)
	{
		descriptor.usage |= WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
		is_storage = true;
	}
	if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD)
	{
		descriptor.usage |= WGPUBufferUsage_CopyDst;
		if(!is_storage)
			descriptor.usage |= WGPUBufferUsage_MapRead;
	}
	if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_UPLOAD)
		descriptor.usage |= WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
	if(buffer->usage & PULSE_INTERNAL_BUFFER_USAGE_UNIFORM_ACCESS)
		descriptor.usage |= WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;

	webgpu_buffer->buffer = wgpuDeviceCreateBuffer(webgpu_device->device, &descriptor);
	if(webgpu_buffer->buffer == PULSE_NULLPTR)
	{
		free(webgpu_buffer);
		free(buffer);
		return PULSE_NULL_HANDLE;
	}
	return buffer;
}

#include <stdio.h>

static void WebGPUMapBufferCallback(WGPUMapAsyncStatus status, WGPUStringView message, void* userdata1, void* userdata2)
{
	atomic_int* mapping_finished = (atomic_int*)userdata1;
	PulseBuffer buffer = (PulseBuffer)userdata2;
	puts("test");
	if(status == WGPUMapAsyncStatus_Success)
		atomic_store(mapping_finished, 1);
	else
	{
		const char* reasons[] = {
			"nvm it was successfull",
			"instance has been dropped",
			"an error occured",
			"mapping was aborted",
		};
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogErrorFmt(buffer->device->backend, "(WebGPU) buffer mapping failed because %s. %.*s", reasons[status], message.length, message.data);
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		atomic_store(mapping_finished, 2);
	}
}

bool WebGPUMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data)
{
	WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(buffer, WebGPUBuffer*);

	// If we only upload we can just use wgpuQueueWriteBuffer
	// https://toji.dev/webgpu-best-practices/buffer-uploads.html
	if(mode == PULSE_MAP_WRITE)
	{
		if(webgpu_buffer->map == PULSE_NULLPTR)
			webgpu_buffer->map = malloc(buffer->size);
		else
			webgpu_buffer->map = realloc(webgpu_buffer->map, buffer->size);
		PULSE_CHECK_ALLOCATION_RETVAL(webgpu_buffer->map, false);
	}
	else
	{
		atomic_int mapping_finished;
		atomic_store(&mapping_finished, 0);

		const uint32_t timeout = 5000;
		clock_t start = clock();

		webgpu_buffer->map = PULSE_NULLPTR;

		WGPUBufferMapCallbackInfo callback_info = { 0 };
		callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
		callback_info.callback = WebGPUMapBufferCallback;
		callback_info.userdata1 = &mapping_finished;
		callback_info.userdata2 = buffer;
		wgpuBufferMapAsync(webgpu_buffer->buffer, WGPUMapMode_Read, 0, buffer->size, callback_info);

		while(atomic_load(&mapping_finished) == 0)
		{
			clock_t elapsed = clock() - start;
			if(elapsed > timeout)
			{
				if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
					PulseLogError(buffer->device->backend, "(WebGPU) buffer mapping failed (timeout)");
				PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
				return false;
			}
			PulseSleep(1); // 1ms
		}

		if(atomic_load(&mapping_finished) == 1)
			webgpu_buffer->map = (void*)wgpuBufferGetConstMappedRange(webgpu_buffer->buffer, 0, WGPU_WHOLE_MAP_SIZE);
	}
	if(webgpu_buffer->map == PULSE_NULLPTR)
		return false;
	webgpu_buffer->current_map_mode = mode;
	*data = webgpu_buffer->map;
	return true;
}

void WebGPUUnmapBuffer(PulseBuffer buffer)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(buffer->device, WebGPUDevice*);
	WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(buffer, WebGPUBuffer*);
	if(webgpu_buffer->current_map_mode == PULSE_MAP_WRITE)
		wgpuQueueWriteBuffer(webgpu_device->queue, webgpu_buffer->buffer, 0, webgpu_buffer->map, buffer->size);
	else
		wgpuBufferUnmap(webgpu_buffer->buffer);
	webgpu_buffer->map = PULSE_NULLPTR;
}

bool WebGPUCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
}

bool WebGPUCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
}

void WebGPUDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	PULSE_UNUSED(device);
	WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(buffer, WebGPUBuffer*);
	wgpuBufferRelease(webgpu_buffer->buffer);
	free(webgpu_buffer);
	free(buffer);
}
