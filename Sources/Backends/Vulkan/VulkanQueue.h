// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_QUEUES_H_
#define PULSE_VULKAN_QUEUES_H_

#include "VulkanEnums.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"

typedef struct VulkanQueue
{
	VulkanDevice* device;
	VkQueue queue;
	int32_t queue_family_index;
} VulkanQueue;

bool VulkanFindPhysicalDeviceQueueFamily(VulkanInstance* instance, VkPhysicalDevice physical, VulkanQueueType type, int32_t* queue_family_index);
bool VulkanPrepareDeviceQueue(VulkanInstance* instance, VulkanDevice* device, VulkanQueueType type);
bool VulkanRetrieveDeviceQueue(VulkanDevice* device, VulkanQueueType type);

#endif // PULSE_VULKAN_QUEUES_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
