// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_H_
#define PULSE_VULKAN_H_

#include <vulkan/vulkan_core.h>

#include "VulkanDevice.h"
#include "VulkanInstance.h"

#include "../../PulseInternal.h"

#define VULKAN_RETRIEVE_DRIVER_DATA_AS(handle, cast) ((cast)handle->driver_data)

#define CHECK_VK_RETVAL(res, error, retval) \
	if((res) != VK_SUCCESS) \
	{ \
		PulseSetInternalError(error); \
		return retval; \
	}

#define CHECK_VK(res, error) CHECK_VK_RETVAL(res, error, )

typedef struct VulkanGlobal
{
	#define PULSE_VULKAN_GLOBAL_FUNCTION(fn) PFN_##fn fn;
		#include "VulkanGlobalPrototypes.h"
	#undef PULSE_VULKAN_GLOBAL_FUNCTION
} VulkanGlobal;

typedef struct VulkanDriverData
{
	VulkanInstance instance;
} VulkanDriverData;

VulkanGlobal* VulkanGetGlobal();

PulseBackendFlags VulkanCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_VULKAN in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_VULKAN_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND