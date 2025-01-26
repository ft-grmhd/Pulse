// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_COMMAND_POOL_H_
#define PULSE_VULKAN_COMMAND_POOL_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "VulkanEnums.h"

typedef struct VulkanCommandPool
{
	PulseDevice device;

	VkCommandPool pool;
	VulkanQueueType queue_type;

	PulseThreadID thread_id;

	PulseCommandList* available_command_lists;
	uint32_t available_command_lists_capacity;
	uint32_t available_command_lists_size;
} VulkanCommandPool;

bool VulkanInitCommandPool(PulseDevice device, VulkanCommandPool* pool, VulkanQueueType queue_type);
void VulkanUninitCommandPool(VulkanCommandPool* pool);

#endif // PULSE_VULKAN_COMMAND_POOL_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
