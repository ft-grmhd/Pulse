// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_BINDGROUPS_H_
#define PULSE_OPENGL_BINDGROUPS_H_

#include <Pulse.h>
#include "OpenGL.h"
#include "OpenGLEnums.h"
#include "../../PulseInternal.h"

#define OPENGL_POOL_SIZE 128

typedef struct OpenGLBindsGroupLayout
{
	union
	{
		struct
		{
			uint32_t storage_buffer_count;
			uint32_t storage_texture_count;
		} ReadOnly;
		struct
		{
			uint32_t storage_buffer_count;
			uint32_t storage_texture_count;
		} ReadWrite;
		struct
		{
			uint32_t buffer_count;
		} Uniform;
	};

	OpenGLBindsGroupType type;
} OpenGLBindsGroupLayout;

typedef struct OpenGLBindsGroup
{
	union
	{
		// NULL terminated arrays
		struct
		{
			PulseBuffer* storage_buffers;
			PulseImage* images;
		} ReadOnly;
		struct
		{
			PulseBuffer* storage_buffers;
			PulseImage* images;
		} ReadWrite;
		struct
		{
			PulseBuffer* buffers;
		} Uniform;
	};

	OpenGLBindsGroupType type;
	OpenGLBindsGroupLayout* layout;
	struct OpenGLBindsGroupPool* pool;
} OpenGLBindsGroup;

typedef struct OpenGLBindsGroupLayoutManager
{
	PulseDevice device;
	OpenGLBindsGroupLayout** layouts;
	uint32_t layouts_capacity;
	uint32_t layouts_size;
} OpenGLBindsGroupLayoutManager;

typedef struct OpenGLBindsGroupPool
{
	OpenGLBindsGroup* used_groups[OPENGL_POOL_SIZE];
	OpenGLBindsGroup* free_groups[OPENGL_POOL_SIZE];
	uint32_t used_index;
	uint32_t free_index;
	PulseDevice device;
	PulseThreadID thread_id;
	uint32_t allocations_count;
} OpenGLBindsGroupPool;

typedef struct OpenGLBindsGroupPoolManager
{
	PulseDevice device;
	OpenGLBindsGroupPool** pools;
	uint32_t pools_capacity;
	uint32_t pools_size;
} OpenGLBindsGroupPoolManager;

void OpenGLInitBindsGroupLayoutManager(OpenGLBindsGroupLayoutManager* manager, PulseDevice device);
OpenGLBindsGroupLayout* OpenGLGetBindsGroupLayout(OpenGLBindsGroupLayoutManager* manager,
														uint32_t read_storage_images_count,
														uint32_t read_storage_buffers_count,
														uint32_t write_storage_images_count,
														uint32_t write_storage_buffers_count,
														uint32_t uniform_buffers_count);
void OpenGLDestroyBindsGroupLayoutManager(OpenGLBindsGroupLayoutManager* manager);

OpenGLBindsGroup* OpenGLRequestBindsGroupFromPool(OpenGLBindsGroupPool* pool, const OpenGLBindsGroupLayout* layout);
void OpenGLReturnBindsGroupToPool(OpenGLBindsGroupPool* pool, const OpenGLBindsGroup* group);

void OpenGLInitBindsGroupPoolManager(OpenGLBindsGroupPoolManager* manager, PulseDevice device);
OpenGLBindsGroupPool* OpenGLGetAvailableBindsGroupPool(OpenGLBindsGroupPoolManager* manager);
void OpenGLDestroyBindsGroupPoolManager(OpenGLBindsGroupPoolManager* manager);

#endif // PULSE_OPENGL_BINDGROUPS_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
