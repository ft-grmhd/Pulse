// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

// No header guard

#ifndef PULSE_VULKAN_GLOBAL_FUNCTION
	#error "You must define PULSE_VULKAN_GLOBAL_FUNCTION before including this file"
#endif

#ifdef VK_VERSION_1_0
	PULSE_VULKAN_GLOBAL_FUNCTION(vkCreateInstance)
	PULSE_VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceExtensionProperties)
	PULSE_VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceLayerProperties)
	PULSE_VULKAN_GLOBAL_FUNCTION(vkGetInstanceProcAddr)
#endif
