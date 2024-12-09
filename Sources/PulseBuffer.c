// Copyright (C) 2024 kanel
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

PULSE_API bool PulseMapBuffer(PulseBuffer buffer, void** data)
{
	PULSE_CHECK_HANDLE_RETVAL(buffer, false);
	PULSE_CHECK_PTR_RETVAL(data, false);
	bool res = buffer->device->PFN_MapBuffer(buffer, data);
	if(res)
		buffer->is_mapped = true;
	return res;
}

PULSE_API void PulseUnmapBuffer(PulseBuffer buffer)
{
	PULSE_CHECK_HANDLE(buffer);
	buffer->device->PFN_UnmapBuffer(buffer);
	buffer->is_mapped = false;
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
}
