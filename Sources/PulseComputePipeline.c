// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseComputePipeline PulseCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	if(info == PULSE_NULLPTR && PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
		PulseLogError(device->backend, "null infos pointer");
	PULSE_CHECK_PTR_RETVAL(info, PULSE_NULL_HANDLE);
	PulseComputePipeline pipeline = device->PFN_CreateComputePipeline(device, info);
	if(pipeline == PULSE_NULL_HANDLE)
		return PULSE_NULL_HANDLE;
	pipeline->num_readonly_storage_images = info->num_readonly_storage_images;
	pipeline->num_readonly_storage_buffers = info->num_readonly_storage_buffers;
	pipeline->num_readwrite_storage_images = info->num_readwrite_storage_buffers;
	pipeline->num_readwrite_storage_buffers = info->num_readwrite_storage_buffers;
	pipeline->num_uniform_buffers = info->num_uniform_buffers;
	return pipeline;
}

PULSE_API void PulseDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	PULSE_CHECK_HANDLE(device);
	device->PFN_DestroyComputePipeline(device, pipeline);
}
