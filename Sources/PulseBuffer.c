// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "PulseDefs.h"
#include "PulseInternal.h"
#include "PulseProfile.h"

PULSE_API PulseBuffer PulseCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(create_infos == PULSE_NULLPTR)
		{
			PulseLogError(device->backend, "create_infos is NULL");
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			return PULSE_NULL_HANDLE;
		}
	}

	PulseBuffer buffer = device->PFN_CreateBuffer(device, create_infos);
	if(buffer == PULSE_NULL_HANDLE)
		return PULSE_NULL_HANDLE;
	PULSE_EXPAND_ARRAY_IF_NEEDED(device->allocated_buffers, PulseBuffer, device->allocated_buffers_size, device->allocated_buffers_capacity, 64);
	device->allocated_buffers[device->allocated_buffers_size] = buffer;
	device->allocated_buffers_size++;
	return buffer;
}

PULSE_API bool PulseMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data)
{
	PULSE_CHECK_HANDLE_RETVAL(buffer, false);
	PULSE_CHECK_PTR_RETVAL(data, false);

	if(buffer->is_mapped)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogError(buffer->device->backend, "buffer is already mapped");
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		return false;
	}

	PulseFlags storage_flags = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE;
	if((buffer->usage & storage_flags) != 0)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogError(buffer->device->backend, "cannot map a buffer that has been created with storage flags");
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		return false;
	}

	PulseFlags transfer_flags = PULSE_BUFFER_USAGE_TRANSFER_UPLOAD | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;
	if((buffer->usage & transfer_flags) == 0)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogError(buffer->device->backend, "cannot map a buffer that has not been created with upload or download flags");
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		return false;
	}

	if((buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_UPLOAD) == 0 && mode == PULSE_MAP_WRITE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogError(buffer->device->backend, "cannot map a buffer that has not been created with upload flags for writting");
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		return false;
	}
	if((buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD) == 0 && mode == PULSE_MAP_READ)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogError(buffer->device->backend, "cannot map a buffer that has not been created with download flags for reading");
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		return false;
	}

	bool res = buffer->device->PFN_MapBuffer(buffer, mode, data);
	if(res)
		buffer->is_mapped = true;
	return res;
}

PULSE_API void PulseUnmapBuffer(PulseBuffer buffer)
{
	PULSE_CHECK_HANDLE(buffer);

	if(!buffer->is_mapped)
	{
		if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(buffer->device->backend))
			PulseLogError(buffer->device->backend, "buffer is not mapped");
		return;
	}

	buffer->device->PFN_UnmapBuffer(buffer);
	buffer->is_mapped = false;
}

PULSE_API bool PulseCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
	PULSE_CHECK_PTR_RETVAL(src, false);
	PULSE_CHECK_HANDLE_RETVAL(src->buffer, false);
	PULSE_CHECK_PTR_RETVAL(dst, false);
	PULSE_CHECK_HANDLE_RETVAL(dst->buffer, false);

	PulseBackend backend = src->buffer->device->backend;

	if(src->buffer->device != dst->buffer->device)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "source buffer has been created on a different device (%p) than the destination buffer (%p)", src->buffer->device, dst->buffer->device);
		PulseSetInternalError(PULSE_ERROR_INVALID_DEVICE);
		return false;
	}

	if(src->size + src->offset > src->buffer->size)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "source buffer region (%lld) is bigger than the buffer size (%lld)", src->size + src->offset, src->buffer->size);
		PulseSetInternalError(PULSE_ERROR_INVALID_REGION);
		return false;
	}

	if(dst->size + dst->offset > dst->buffer->size)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "destination buffer region (%lld) is bigger than the buffer size (%lld)", dst->size + dst->offset, dst->buffer->size);
		PulseSetInternalError(PULSE_ERROR_INVALID_REGION);
		return false;
	}

	if(src->buffer == dst->buffer)
		return true;

	return src->buffer->device->PFN_CopyBufferToBuffer(cmd, src, dst);
}

PULSE_API bool PulseCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
	PULSE_CHECK_HANDLE_RETVAL(src, false);
	PULSE_CHECK_PTR_RETVAL(dst, false);
	PULSE_CHECK_HANDLE_RETVAL(dst->image, false);

	PulseBackend backend = src->buffer->device->backend;

	if(src->buffer->device != dst->image->device)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "source buffer has been created on a different device (%p) than the destination buffer (%p)", src->buffer->device, dst->image->device);
		PulseSetInternalError(PULSE_ERROR_INVALID_DEVICE);
		return false;
	}

	if(src->size + src->offset > src->buffer->size)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "source buffer region (%lld) is bigger than the buffer size (%lld)", src->size + src->offset, src->buffer->size);
		PulseSetInternalError(PULSE_ERROR_INVALID_REGION);
		return false;
	}

	if(dst->width > dst->image->width)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "destination image region width (%lld) is bigger than image width (%lld)", dst->width, dst->image->width);
		PulseSetInternalError(PULSE_ERROR_INVALID_REGION);
		return false;
	}

	if(dst->height > dst->image->height)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "destination image region height (%lld) is bigger than image height (%lld)", dst->height, dst->image->height);
		PulseSetInternalError(PULSE_ERROR_INVALID_REGION);
		return false;
	}

	return src->buffer->device->PFN_CopyBufferToImage(cmd, src, dst);
}

PULSE_API void PulseDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	PULSE_CHECK_HANDLE(device);

	if(buffer == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "buffer is NULL, this may be a bug in your application");
		return;
	}
	if(buffer->is_mapped)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "buffer is still mapped, consider unmapping it before destroy");
	}
	if(buffer->device != device)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogErrorFmt(device->backend, "cannot destroy buffer [%p] that have been allocated with device [%p] using device [%p]", buffer, buffer->device, device);
		PulseSetInternalError(PULSE_ERROR_INVALID_DEVICE);
		return;
	}
	for(uint32_t i = 0; i < device->allocated_buffers_size; i++)
	{
		if(device->allocated_buffers[i] == buffer)
		{
			PULSE_DEFRAG_ARRAY(device->allocated_buffers, device->allocated_buffers_size, i);
			break;
		}
	}
	device->PFN_DestroyBuffer(device, buffer);
	device->allocated_buffers_size--;
}
