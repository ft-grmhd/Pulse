// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Vulkan.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"

bool VulkanInitCommandPool(PulseDevice device, VulkanCommandPool* pool, VulkanQueueType queue_type)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	PULSE_CHECK_PTR_RETVAL(pool, false);

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);

	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = vulkan_device->queues[queue_type]->queue_family_index;
	create_info.pNext = PULSE_NULLPTR;
	CHECK_VK_RETVAL(vulkan_device->vkCreateCommandPool(vulkan_device->device, &create_info, PULSE_NULLPTR, &pool->pool), PULSE_ERROR_INITIALIZATION_FAILED, false);

	pool->thread_id = PulseGetThreadID();

	pool->available_command_lists = PULSE_NULLPTR;
	pool->available_command_lists_capacity = 0;
	pool->available_command_lists_size = 0;

	pool->device = device;

	return true;
}

void VulkanUninitCommandPool(VulkanCommandPool* pool)
{
	PULSE_CHECK_PTR(pool);

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pool->device, VulkanDevice*);
	vulkan_device->vkDestroyCommandPool(vulkan_device->device, pool->pool, PULSE_NULLPTR);
	if(pool->available_command_lists != PULSE_NULLPTR)
		free(pool->available_command_lists);
	pool->thread_id = 0;
	pool->available_command_lists = PULSE_NULLPTR;
	pool->available_command_lists_capacity = 0;
	pool->available_command_lists_size = 0;
}
