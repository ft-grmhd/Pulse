// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_H_
#define PULSE_VULKAN_H_

#include <vulkan/vulkan_core.h>

#include "VulkanDevice.h"
#include "VulkanInstance.h"

typedef struct VulkanGlobal
{
	#define PULSE_VULKAN_GLOBAL_FUNCTION(fn) PFN_##fn fn;
		#include "VulkanGlobalPrototypes.h"
	#undef PULSE_VULKAN_GLOBAL_FUNCTION
} VulkanGlobal;

typedef struct VulkanPulseDevice
{
	VulkanInstance instance;
	VulkanDevice device;
} VulkanPulseDevice;

VulkanGlobal* VulkanGetGlobal();

#endif // PULSE_VULKAN_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
