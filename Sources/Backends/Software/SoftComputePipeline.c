// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftDevice.h"
#include "SoftComputePipeline.h"

void SoftAllocateWorkgroupMemory(struct spvm_state* state, spvm_word result_id, spvm_word type_id)
{
	SoftComputePipeline* pipeline = (SoftComputePipeline*)state->owner->user_data;
	mtx_lock(&pipeline->workgroup_memory_allocations_mutex);
		spvm_result_t result = &state->results[result_id];
		PULSE_EXPAND_ARRAY_IF_NEEDED(pipeline->workgroup_memory_allocations, SoftSharedMemoryEntry, pipeline->workgroup_memory_allocations_size, pipeline->workgroup_memory_allocations_capacity, 16);
		SoftSharedMemoryEntry* entry = &pipeline->workgroup_memory_allocations[pipeline->workgroup_memory_allocations_size];
		entry->destination = &state->results[result_id];
		entry->slot = result_id;
		memcpy(&entry->data, entry->destination, sizeof(spvm_result));
		spvm_result_allocate_typed_value(result, state->results, type_id);
		pipeline->workgroup_memory_allocations_size++;
	mtx_unlock(&pipeline->workgroup_memory_allocations_mutex);
}
/*
void SoftWriteWorkgroupMemory(struct spvm_state* state, spvm_word result_id, spvm_word val_id)
{
	spvm_result_t ptr = &state->results[result_id];
	if(ptr->source_location != PULSE_NULLPTR)
	{
		spvm_word word_count = ptr->source_word_count;
		spvm_source code = ptr->source_location;

		spvm_word var_type = SPVM_READ_WORD(code);
		spvm_word id = SPVM_READ_WORD(code);
		spvm_word memory_id = SPVM_READ_WORD(code);

		spvm_result_t sharedData = PULSE_NULLPTR;

		ed::DebugInformation* dbgr = (ed::DebugInformation*)state->owner->user_data;

		for(int i = 0; i < dbgr->SharedMemory.size(); i++)
		{
			if(dbgr->SharedMemory[i].Slot == memory_id)
			{
				sharedData = &dbgr->SharedMemory[i].Data;
				break;
			}
		}

		if(sharedData == PULSE_NULLPTR)
			return;

		spvm_word index_count = word_count - 4;

		spvm_word index_id = SPVM_READ_WORD(code);
		spvm_word index = state->results[index_id].members[0].value.s;

		spvm_member_t result = sharedData->members + MIN(index, sharedData->member_count - 1);

		while(index_count)
		{
			index_id = SPVM_READ_WORD(code);
			index = state->results[index_id].members[0].value.s;

			result = result->members + MIN(index, result->member_count - 1);

			index_count--;
		}

		spvm_member* members = PULSE_NULLPTR;
		spvm_word member_count = 0;
		if(result->member_count != 0)
		{
			member_count = result->member_count;
			members = result->members;
		}
		else
		{
			member_count = 1;
			members = result;
		}
		spvm_member_memcpy(members, state->results[val_id].members, member_count);
	}
}

void SoftControlBarrier(struct spvm_state* state, spvm_word exec, spvm_word mem, spvm_word sem)
{
	ed::DebugInformation* dbgr = (ed::DebugInformation*)state->owner->user_data;

	// copy memory
	for(int i = 0; i < dbgr->SharedMemory.size(); i++)
	{
		const ed::DebugInformation::SharedMemoryEntry& entry = dbgr->SharedMemory[i];
		spvm_member_memcpy(entry.Data.members, entry.Destination->members, entry.Data.member_count);
	}

	// synchronize threads
	dbgr->SyncWorkgroup();

	// copy memory
	for(int i = 0; i < dbgr->SharedMemory.size(); i++)
	{
		const ed::DebugInformation::SharedMemoryEntry& entry = dbgr->SharedMemory[i];
		spvm_member_memcpy(entry.Destination->members, entry.Data.members, entry.Destination->member_count);
	}
}
*/

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
	PULSE_CHECK_ALLOCATION_RETVAL(soft_pipeline->entry_point, PULSE_NULL_HANDLE);
	strcpy((char*)soft_pipeline->entry_point, info->entrypoint);

	soft_pipeline->program->user_data = soft_pipeline;
	soft_pipeline->program->allocate_workgroup_memory = SoftAllocateWorkgroupMemory;

	// Create dummy state to retrieve informations from the spirv
	spvm_state_t state = spvm_state_create(soft_pipeline->program);
	spvm_state_delete(state);

	mtx_init(&soft_pipeline->workgroup_memory_allocations_mutex, mtx_plain);

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
	mtx_destroy(&soft_pipeline->workgroup_memory_allocations_mutex);
	free(soft_pipeline->workgroup_memory_allocations);
	free((void*)soft_pipeline->entry_point);
	free(soft_pipeline);
	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Soft) destroyed compute pipeline %p", pipeline);
	free(pipeline);
}
