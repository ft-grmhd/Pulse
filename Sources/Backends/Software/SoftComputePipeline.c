// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftDevice.h"
#include "SoftComputePipeline.h"

PulseComputePipeline SoftCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	SoftDevice* soft_device = SOFT_RETRIEVE_DRIVER_DATA_AS(device, SoftDevice*);

	PulseComputePipelineHandler* pipeline = (PulseComputePipelineHandler*)calloc(1, sizeof(PulseComputePipelineHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pipeline, PULSE_NULL_HANDLE);

	SoftComputePipeline* soft_pipeline = (SoftComputePipeline*)calloc(1, sizeof(SoftComputePipeline));
	PULSE_CHECK_ALLOCATION_RETVAL(soft_pipeline, PULSE_NULL_HANDLE);

	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(info->code == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid code pointer passed to PulseComputePipelineCreateInfo");
		if(info->entrypoint == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid entrypoint pointer passed to PulseComputePipelineCreateInfo");
		if(info->format == PULSE_SHADER_FORMAT_SPIRV_BIT && (device->backend->supported_shader_formats & PULSE_SHADER_FORMAT_SPIRV_BIT) == 0)
			PulseLogError(device->backend, "invalid shader format passed to PulseComputePipelineCreateInfo");
	}

	soft_pipeline->program = spvm_program_create(soft_device->spv_context, (spvm_source)info->code, info->code_size / sizeof(spvm_word));
	soft_pipeline->entry_point = calloc(1, strlen(info->entrypoint));
	strcpy((char*)soft_pipeline->entry_point, info->entrypoint);

	// Create dummy state to retrieve informations from the spirv
	spvm_state_t state = spvm_state_create(soft_pipeline->program);
	spvm_state_delete(state);

	pipeline->driver_data = soft_pipeline;

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Soft) created new compute pipeline %p", pipeline);
	return pipeline;
}

void SoftDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	if(pipeline == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "compute pipeline is NULL, this may be a bug in your application");
		return;
	}
	PULSE_UNUSED(device);
	SoftComputePipeline* soft_pipeline = SOFT_RETRIEVE_DRIVER_DATA_AS(pipeline, SoftComputePipeline*);
	spvm_program_delete(soft_pipeline->program);
	free(soft_pipeline->entry_point);
	free(soft_pipeline);
	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Soft) destroyed compute pipeline %p", pipeline);
	free(pipeline);
}
