// Copyright (C) 2025 kanel
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

#define CHECK_VK_RETVAL(backend, res, error, retval) \
	do { \
		if((res) != VK_SUCCESS) \
		{ \
			if(backend != PULSE_NULL_HANDLE && PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend)) \
				PulseLogErrorFmt(backend, "(Vulkan) call to a Vulkan function failed due to %s", VulkanVerbaliseResult(res)); \
			PulseSetInternalError(error); \
			return retval; \
		} \
	} while(0) \

#define CHECK_VK(backend, res, error) CHECK_VK_RETVAL(backend, res, error, )

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

const char* VulkanVerbaliseResult(VkResult res);

PulseBackendFlags VulkanCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_VULKAN in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_VULKAN_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
