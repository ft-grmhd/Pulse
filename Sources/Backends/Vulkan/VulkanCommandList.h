// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_COMMAND_LIST_H_
#define PULSE_VULKAN_COMMAND_LIST_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "VulkanCommandPool.h"

typedef struct VulkanCommandList
{
	PulseDevice device;
	VulkanCommandPool* pool;
	PulseThreadID thread_id;
	VkCommandBuffer cmd;

	PulseComputePipeline* compute_pipelines_bound;
	uint32_t compute_pipelines_bound_capacity;
	uint32_t compute_pipelines_bound_size;
} VulkanCommandList;

void VulkanInitCommandList(VulkanCommandPool* pool);

#endif // PULSE_VULKAN_COMMAND_LIST_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
