// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <Pulse.h>

#include "OpenGL.h"
#include "OpenGLBindsGroup.h"
#include "OpenGLDevice.h"

void OpenGLInitBindsGroupLayoutManager(OpenGLBindsGroupLayoutManager* manager, PulseDevice device)
{
	memset(manager, 0, sizeof(OpenGLBindsGroupLayoutManager));
	manager->device = device;
}

OpenGLBindsGroupLayout* OpenGLGetBindsGroupLayout(OpenGLBindsGroupLayoutManager* manager,
														uint32_t read_storage_images_count,
														uint32_t read_storage_buffers_count,
														uint32_t write_storage_images_count,
														uint32_t write_storage_buffers_count,
														uint32_t uniform_buffers_count)
{
	for(uint32_t i = 0; i < manager->layouts_size; i++)
	{
		OpenGLBindsGroupLayout* layout = manager->layouts[i];
		if( layout->ReadOnly.storage_buffer_count == read_storage_buffers_count &&
			layout->ReadOnly.storage_texture_count == read_storage_images_count &&
			layout->ReadWrite.storage_buffer_count == write_storage_buffers_count &&
			layout->ReadWrite.storage_texture_count == write_storage_images_count &&
			layout->Uniform.buffer_count == uniform_buffers_count)
		{
			return layout;
		}
	}

	PULSE_EXPAND_ARRAY_IF_NEEDED(manager->layouts, OpenGLBindsGroupLayout*, manager->layouts_size, manager->layouts_capacity, 1);
	PULSE_CHECK_ALLOCATION_RETVAL(manager->layouts, PULSE_NULLPTR);

	OpenGLBindsGroupLayout* layout = (OpenGLBindsGroupLayout*)malloc(sizeof(OpenGLBindsGroupLayout));
	manager->layouts[manager->layouts_size] = layout;
	manager->layouts_size++;

	if(uniform_buffers_count != 0)
		layout->type = OPENGL_BINDS_GROUP_UNIFORM;
	else if(write_storage_images_count != 0 || write_storage_buffers_count != 0)
		layout->type = OPENGL_BINDS_GROUP_READ_WRITE;
	else
		layout->type = OPENGL_BINDS_GROUP_READ_ONLY;

	layout->ReadOnly.storage_buffer_count = read_storage_buffers_count;
	layout->ReadOnly.storage_texture_count = read_storage_images_count;
	layout->ReadWrite.storage_buffer_count = write_storage_buffers_count;
	layout->ReadWrite.storage_texture_count = write_storage_images_count;
	layout->Uniform.buffer_count = uniform_buffers_count;

	return layout;
}

void OpenGLDestroyBindsGroupLayoutManager(OpenGLBindsGroupLayoutManager* manager)
{
	for(uint32_t i = 0; i < manager->layouts_size; i++)
		free(manager->layouts[i]);
	free(manager->layouts);
	memset(manager, 0, sizeof(OpenGLBindsGroupPoolManager));
}

OpenGLBindsGroup* OpenGLRequestBindsGroupFromPool(OpenGLBindsGroupPool* pool, const OpenGLBindsGroupLayout* layout)
{
	for(size_t i = 0; i < OPENGL_POOL_SIZE; i++)
	{
		if(pool->free_groups[i] == PULSE_NULLPTR)
			break; // Due to defragmentation we are sure not to find valid groups after the first NULL one
		if(pool->free_groups[i]->layout == layout)
		{
			OpenGLBindsGroup* group = pool->free_groups[i];
			PULSE_DEFRAG_ARRAY(pool->free_groups, OPENGL_POOL_SIZE, i);
			pool->free_index--;
			pool->used_groups[pool->used_index] = group;
			pool->used_index++;
			return group;
		}
	}

	OpenGLBindsGroup* group = (OpenGLBindsGroup*)calloc(1, sizeof(OpenGLBindsGroup));
	PULSE_CHECK_ALLOCATION_RETVAL(group, PULSE_NULLPTR);

	group->layout = (OpenGLBindsGroupLayout*)layout;
	group->pool = pool;
	group->type = layout->type;
	if(layout->type == OPENGL_BINDS_GROUP_READ_ONLY)
	{
		group->ReadOnly.storage_buffers = (PulseBuffer*)calloc(layout->ReadOnly.storage_buffer_count + 1, sizeof(PulseBuffer));
		group->ReadOnly.images = (PulseImage*)calloc(layout->ReadOnly.storage_texture_count + 1, sizeof(PulseImage));
	}
	else if(layout->type == OPENGL_BINDS_GROUP_READ_WRITE)
	{
		group->ReadWrite.storage_buffers = (PulseBuffer*)calloc(layout->ReadWrite.storage_buffer_count + 1, sizeof(PulseBuffer));
		group->ReadWrite.images = (PulseImage*)calloc(layout->ReadWrite.storage_texture_count + 1, sizeof(PulseImage));
	}
	else if(layout->type == OPENGL_BINDS_GROUP_UNIFORM)
		group->Uniform.buffers = (PulseBuffer*)calloc(layout->Uniform.buffer_count + 1, sizeof(PulseBuffer));

	pool->used_groups[pool->used_index] = group;
	pool->used_index++;

	pool->allocations_count++;
	return group;
}

void OpenGLReturnBindsGroupToPool(OpenGLBindsGroupPool* pool, const OpenGLBindsGroup* group)
{
	for(size_t i = 0; i < OPENGL_POOL_SIZE; i++)
	{
		if(pool->used_groups[i] == PULSE_NULLPTR)
			break; // Due to defragmentation we are sure not to find valid groups after the first NULL one
		if(pool->used_groups[i] == group)
		{
			PULSE_DEFRAG_ARRAY(pool->used_groups, OPENGL_POOL_SIZE, i);
			pool->used_index--;
			pool->free_groups[pool->free_index] = (OpenGLBindsGroup*)group;
			pool->free_index++;
			return;
		}
	}
}

void OpenGLInitBindsGroupPool(OpenGLBindsGroupPool* pool, PulseDevice device)
{
	memset(pool, 0, sizeof(OpenGLBindsGroupPool));
	pool->device = device;
	pool->thread_id = PulseGetThreadID();
}

void OpenGLDestroyBindsGroup(OpenGLBindsGroup* group)
{
	if(group->type == OPENGL_BINDS_GROUP_READ_ONLY)
	{
		free(group->ReadOnly.storage_buffers);
		free(group->ReadOnly.images);
	}
	else if(group->type == OPENGL_BINDS_GROUP_READ_WRITE)
	{
		free(group->ReadWrite.storage_buffers);
		free(group->ReadWrite.images);
	}
	else if(group->type == OPENGL_BINDS_GROUP_UNIFORM)
		free(group->Uniform.buffers);
}

void OpenGLDestroyBindsGroupPool(OpenGLBindsGroupPool* pool)
{
	for(size_t i = 0; i < OPENGL_POOL_SIZE; i++)
	{
		if(pool->used_groups[i] != PULSE_NULLPTR)
		{
			OpenGLDestroyBindsGroup(pool->used_groups[i]);
			free(pool->used_groups[i]);
			pool->used_groups[i] = PULSE_NULLPTR;
		}
		if(pool->free_groups[i] != PULSE_NULLPTR)
		{
			OpenGLDestroyBindsGroup(pool->free_groups[i]);
			free(pool->free_groups[i]);
			pool->free_groups[i] = PULSE_NULLPTR;
		}
		if(pool->free_groups[i] == PULSE_NULLPTR && pool->used_groups[i] == PULSE_NULLPTR)
			break; // Due to defragmentation we are sure not to find valid groups after the first NULL one
	}
	memset(pool, 0, sizeof(OpenGLBindsGroupPool));
}

void OpenGLInitBindsGroupPoolManager(OpenGLBindsGroupPoolManager* manager, PulseDevice device)
{
	memset(manager, 0, sizeof(OpenGLBindsGroupPoolManager));
	manager->device = device;
}

OpenGLBindsGroupPool* OpenGLGetAvailableBindsGroupPool(OpenGLBindsGroupPoolManager* manager)
{
	PulseThreadID thread_id = PulseGetThreadID();
	for(uint32_t i = 0; i < manager->pools_size; i++)
	{
		if(thread_id == manager->pools[i]->thread_id && (manager->pools[i]->allocations_count < OPENGL_POOL_SIZE || manager->pools[i]->free_groups[0] != PULSE_NULLPTR))
			return manager->pools[i];
	}
	PULSE_EXPAND_ARRAY_IF_NEEDED(manager->pools, OpenGLBindsGroupPool*, manager->pools_size, manager->pools_capacity, 1);
	PULSE_CHECK_ALLOCATION_RETVAL(manager->pools, PULSE_NULLPTR);

	manager->pools[manager->pools_size] = (OpenGLBindsGroupPool*)calloc(1, sizeof(OpenGLBindsGroupPool));
	PULSE_CHECK_ALLOCATION_RETVAL(manager->pools[manager->pools_size], PULSE_NULLPTR);

	OpenGLInitBindsGroupPool(manager->pools[manager->pools_size], manager->device);
	manager->pools_size++;
	return manager->pools[manager->pools_size - 1];
}

void OpenGLDestroyBindsGroupPoolManager(OpenGLBindsGroupPoolManager* manager)
{
	for(uint32_t i = 0; i < manager->pools_size; i++)
	{
		OpenGLDestroyBindsGroupPool(manager->pools[i]);
		free(manager->pools[i]);
	}
	free(manager->pools);
	memset(manager, 0, sizeof(OpenGLBindsGroupPoolManager));
}
