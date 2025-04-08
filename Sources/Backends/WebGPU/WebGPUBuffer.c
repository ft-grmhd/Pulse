// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdatomic.h>
#include <string.h>
#include <time.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "WebGPUBuffer.h"
#include "WebGPUImage.h"
#include "WebGPUCommandList.h"

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
	if(buffer->usage & PULSE_INTERNAL_BUFFER_USAGE_PURE_TRANSFER)
		descriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
	else
	{
		if(buffer->usage & PULSE_BUFFER_USAGE_STORAGE_READ || buffer->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE)
		{
			descriptor.usage |= WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
			is_storage = true;
		}
		if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD)
		{
			descriptor.usage |= WGPUBufferUsage_CopyDst;
			if(!is_storage)
			{
				descriptor.usage |= WGPUBufferUsage_MapRead;
				webgpu_buffer->needs_staging_upload = true;
			}
		}
		if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_UPLOAD && !webgpu_buffer->needs_staging_upload)
			descriptor.usage |= WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst;
		if(buffer->usage & PULSE_INTERNAL_BUFFER_USAGE_UNIFORM_ACCESS)
			descriptor.usage |= WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
	}

	webgpu_buffer->buffer = wgpuDeviceCreateBuffer(webgpu_device->device, &descriptor);
	if(webgpu_buffer->buffer == PULSE_NULLPTR)
	{
		free(webgpu_buffer);
		free(buffer);
		return PULSE_NULL_HANDLE;
	}
	return buffer;
}

static void WebGPUMapBufferCallback(WGPUMapAsyncStatus status, WGPUStringView message, void* userdata1, void* userdata2)
{
	atomic_int* mapping_finished = (atomic_int*)userdata1;
	PulseBuffer buffer = (PulseBuffer)userdata2;
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
			PulseLogErrorFmt(buffer->device->backend, "(WebGPU) buffer mapping failed because %s. %.*s", reasons[status - 1], message.length, message.data);
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
		webgpu_buffer->map = malloc(buffer->size);
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
			WebGPUDeviceTick(buffer->device);
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
	{
		wgpuQueueWriteBuffer(webgpu_device->queue, webgpu_buffer->buffer, 0, webgpu_buffer->map, buffer->size);
		free(webgpu_buffer->map);
	}
	else
		wgpuBufferUnmap(webgpu_buffer->buffer);
	webgpu_buffer->map = PULSE_NULLPTR;
}

bool WebGPUCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
	WebGPUBuffer* webgpu_src_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(src->buffer, WebGPUBuffer*);
	WebGPUBuffer* webgpu_dst_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(dst->buffer, WebGPUBuffer*);
	WebGPUCommandList* webgpu_cmd = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd, WebGPUCommandList*);
	if(webgpu_src_buffer->needs_staging_upload)
	{
		if(webgpu_src_buffer->upload_staging == PULSE_NULL_HANDLE)
		{
			PulseBufferCreateInfo buffer_create_info = { 0 };
			buffer_create_info.size = src->buffer->size;
			buffer_create_info.usage = PULSE_INTERNAL_BUFFER_USAGE_PURE_TRANSFER;
			webgpu_src_buffer->upload_staging = WebGPUCreateBuffer(cmd->device, &buffer_create_info);
		}
		void* read_map;
		void* write_map;
		if(WebGPUMapBuffer(src->buffer, PULSE_MAP_READ, &read_map) && WebGPUMapBuffer(webgpu_src_buffer->upload_staging, PULSE_MAP_WRITE, &write_map))
		{
			memcpy(write_map, read_map, src->buffer->size);
			WebGPUUnmapBuffer(src->buffer);
			WebGPUUnmapBuffer(webgpu_src_buffer->upload_staging);
		}
		WebGPUBuffer* webgpu_staging_src_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(webgpu_src_buffer->upload_staging, WebGPUBuffer*);
		wgpuCommandEncoderCopyBufferToBuffer(webgpu_cmd->encoder, webgpu_staging_src_buffer->buffer, src->offset, webgpu_dst_buffer->buffer, dst->offset, (src->size < dst->size ? src->size : dst->size));
	}
	else
		wgpuCommandEncoderCopyBufferToBuffer(webgpu_cmd->encoder, webgpu_src_buffer->buffer, src->offset, webgpu_dst_buffer->buffer, dst->offset, (src->size < dst->size ? src->size : dst->size));
	return true;
}

bool WebGPUCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd->device, WebGPUDevice*);
	WebGPUBuffer* webgpu_src_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(src->buffer, WebGPUBuffer*);
	WebGPUImage* webgpu_dst_image = WEBGPU_RETRIEVE_DRIVER_DATA_AS(dst->image, WebGPUImage*);
	WebGPUCommandList* webgpu_cmd = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd, WebGPUCommandList*);

	PulseImageFormat format = dst->image->format;
	uint32_t block_height = WebGPUGetImageBlockHeight(format) > 1 ? WebGPUGetImageBlockHeight(format) : 1;
	uint32_t blocks_per_column = (dst->image->height + block_height - 1) / block_height;
	uint32_t bytes_per_row = WebGPUBytesPerRow(dst->image->width, dst->image->format);

	if(bytes_per_row == 0)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(cmd->device->backend))
			PulseLogError(cmd->device->backend, "(WebGPU) unsupported image format");
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		return false;
	}

	WGPUTexelCopyBufferLayout layout = { 0 };
	layout.bytesPerRow = bytes_per_row;
	layout.rowsPerImage = blocks_per_column;

	WGPUTexelCopyTextureInfo texture_copy_info = { 0 };
	texture_copy_info.texture = webgpu_dst_image->texture;
	texture_copy_info.mipLevel = 1;
	texture_copy_info.aspect = WGPUTextureAspect_All;
	texture_copy_info.origin.x = dst->x;
	texture_copy_info.origin.y = dst->y;
	texture_copy_info.origin.z = dst->z;

	WGPUExtent3D extent = { 0 };
	extent.width = dst->width;
	extent.height = dst->height;
	extent.depthOrArrayLayers = dst->depth;

	WGPUBuffer buffer;

	if(webgpu_src_buffer->needs_staging_upload)
	{
		if(webgpu_src_buffer->upload_staging == PULSE_NULL_HANDLE)
		{
			PulseBufferCreateInfo buffer_create_info = { 0 };
			buffer_create_info.size = src->buffer->size;
			buffer_create_info.usage = PULSE_INTERNAL_BUFFER_USAGE_PURE_TRANSFER;
			webgpu_src_buffer->upload_staging = WebGPUCreateBuffer(cmd->device, &buffer_create_info);
		}
		void* read_map;
		void* write_map;
		if(WebGPUMapBuffer(src->buffer, PULSE_MAP_READ, &read_map) && WebGPUMapBuffer(webgpu_src_buffer->upload_staging, PULSE_MAP_WRITE, &write_map))
		{
			memcpy(write_map, read_map, src->buffer->size);
			WebGPUUnmapBuffer(src->buffer);
			WebGPUUnmapBuffer(webgpu_src_buffer->upload_staging);
		}
		WebGPUBuffer* webgpu_staging_src_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(webgpu_src_buffer->upload_staging, WebGPUBuffer*);
		buffer = webgpu_staging_src_buffer->buffer;
	}
	else
		buffer = webgpu_src_buffer->buffer;

	if(bytes_per_row >= 256 && bytes_per_row % 256 == 0)
	{
		WGPUTexelCopyBufferInfo buffer_copy_info = { 0 };
		buffer_copy_info.buffer = buffer;
		buffer_copy_info.layout = layout;
		wgpuCommandEncoderCopyBufferToTexture(webgpu_cmd->encoder, &buffer_copy_info, &texture_copy_info, &extent);
	}
	else
		wgpuQueueWriteTexture(webgpu_device->queue, &texture_copy_info, buffer, src->size, &layout, &extent);

	return true;
}

void WebGPUDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	PULSE_UNUSED(device);
	WebGPUBuffer* webgpu_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(buffer, WebGPUBuffer*);
	if(webgpu_buffer->upload_staging != PULSE_NULL_HANDLE)
		WebGPUDestroyBuffer(device, webgpu_buffer->upload_staging);
	wgpuBufferRelease(webgpu_buffer->buffer);
	free(webgpu_buffer);
	free(buffer);
}
