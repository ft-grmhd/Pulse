// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_DEVICE_H_
#define PULSE_VULKAN_DEVICE_H_

#include <vulkan/vulkan_core.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

#include <Pulse.h>

#include "VulkanEnums.h"
#include "VulkanDescriptor.h"
#include "VulkanCommandPool.h"

struct VulkanQueue;

typedef struct VulkanDevice
{
	VulkanDescriptorSetPoolManager descriptor_set_pool_manager;
	VulkanDescriptorSetLayoutManager descriptor_set_layout_manager;

	VulkanCommandPool* cmd_pools;
	uint32_t cmd_pools_size;

	struct VulkanQueue* queues[VULKAN_QUEUE_END_ENUM];

	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDevice physical;

	VkDevice device;

	VmaAllocator allocator;

	#define PULSE_VULKAN_DEVICE_FUNCTION(fn) PFN_##fn fn;
		#include "VulkanDevicePrototypes.h"
	#undef PULSE_VULKAN_DEVICE_FUNCTION
} VulkanDevice;

PulseDevice VulkanCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count);
void VulkanDestroyDevice(PulseDevice device);
VulkanCommandPool* VulkanRequestCmdPoolFromDevice(PulseDevice device, VulkanQueueType queue_type);

#endif // PULSE_VULKAN_DEVICE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
