// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_INSTANCE_H_
#define PULSE_VULKAN_INSTANCE_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>

typedef struct VulkanInstance
{
	#define PULSE_VULKAN_INSTANCE_FUNCTION(fn) PFN_##fn fn;
		#include "VulkanInstancePrototypes.h"
	#undef PULSE_VULKAN_INSTANCE_FUNCTION

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	bool validation_layers_enabled;
} VulkanInstance;

bool VulkanInitInstance(PulseBackend backend, VulkanInstance* instance, PulseDebugLevel debug_level);
void VulkanDestroyInstance(VulkanInstance* instance);

#endif // PULSE_VULKAN_INSTANCE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
