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

typedef struct VulkanDevice
{
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

#endif // PULSE_VULKAN_DEVICE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
