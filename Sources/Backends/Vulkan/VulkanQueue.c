// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "VulkanQueue.h"
#include "Vulkan.h"

#include <stdlib.h>

bool VulkanFindPhysicalDeviceQueueFamily(VulkanInstance* instance, VkPhysicalDevice physical, VulkanQueueType type, int32_t* queue_family_index)
{
	if(physical == VK_NULL_HANDLE)
		return false;

	uint32_t queue_family_count;
	instance->vkGetPhysicalDeviceQueueFamilyProperties(physical, &queue_family_count, PULSE_NULLPTR);
	VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)calloc(queue_family_count, sizeof(VkQueueFamilyProperties));
	if(!queue_families)
		return false;
	instance->vkGetPhysicalDeviceQueueFamilyProperties(physical, &queue_family_count, queue_families);

	bool found = false;
	for(uint32_t i = 0; i < queue_family_count; i++)
	{
		if(type == VULKAN_QUEUE_COMPUTE)
		{
			// try to find a queue that's only for compute
			if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT && (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0)
			{
				*queue_family_index = i;
				found = true;
				break;
			}
			if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) // else just find a compute queue
			{
				*queue_family_index = i;
				found = true;
				break;
			}
		}
		else if(type == VULKAN_QUEUE_TRANSFER)
		{
			if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT && (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
			{
				*queue_family_index = i;
				found = true;
				break;
			}
			if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				*queue_family_index = i;
				found = true;
				break;
			}
		}
	}
	free(queue_families);
	return found;
}

bool VulkanPrepareDeviceQueue(VulkanInstance* instance, VulkanDevice* device, VulkanQueueType type)
{
	if(device == PULSE_NULLPTR)
		return false;
	device->queues[(int)type] = (VulkanQueue*)malloc(sizeof(VulkanQueue));
	if(!device->queues[(int)type])
		return false;
	VulkanQueue* queue = device->queues[(int)type];
	if(!queue)
		return false;
	return VulkanFindPhysicalDeviceQueueFamily(instance, device->physical, type, &queue->queue_family_index);
}

bool VulkanRetrieveDeviceQueue(VulkanDevice* device, VulkanQueueType type)
{
	if(device == PULSE_NULLPTR)
		return false;
	VulkanQueue* queue = device->queues[(int)type];
	if(!queue)
		return false;
	device->vkGetDeviceQueue(device->device, queue->queue_family_index, 0, &queue->queue);
	return true;
}
