// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_H_
#define PULSE_VULKAN_H_

#include <vulkan/vulkan_core.h>

#include "VulkanDevice.h"
#include "VulkanInstance.h"

#define VULKAN_RETRIEVE_DRIVER_DATA(device) ((VulkanDriverData*)device->driver_data)

typedef struct VulkanGlobal
{
	#define PULSE_VULKAN_GLOBAL_FUNCTION(fn) PFN_##fn fn;
		#include "VulkanGlobalPrototypes.h"
	#undef PULSE_VULKAN_GLOBAL_FUNCTION
} VulkanGlobal;

typedef struct VulkanDriverData
{
	VulkanInstance instance;
	VulkanDevice device;
} VulkanDriverData;

VulkanGlobal* VulkanGetGlobal();

PulseBackendFlags VulkanCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_VULKAN in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_VULKAN_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
