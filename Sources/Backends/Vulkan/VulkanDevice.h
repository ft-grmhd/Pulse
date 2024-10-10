// Copyright (C) 2024 kanel
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

struct VulkanQueue;

typedef struct VulkanDevice
{
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

#endif // PULSE_VULKAN_DEVICE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND