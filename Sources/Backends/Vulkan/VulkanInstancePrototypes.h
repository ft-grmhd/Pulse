// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

// No header guard

#ifndef PULSE_VULKAN_INSTANCE_FUNCTION
	#error "You must define PULSE_VULKAN_INSTANCE_FUNCTION before including this file"
#endif

#ifdef VK_VERSION_1_0
	PULSE_VULKAN_INSTANCE_FUNCTION(vkCreateDevice)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkDestroyInstance)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkEnumerateDeviceExtensionProperties)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetDeviceProcAddr)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFormatProperties)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceImageFormatProperties)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties)
	PULSE_VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)
#endif
