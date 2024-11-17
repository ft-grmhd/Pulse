// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdlib.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "Vulkan.h"
#include "VulkanLoader.h"
#include "VulkanInstance.h"

VulkanGlobal* VulkanGetGlobal()
{
	static VulkanGlobal vulkan = { 0 };
	return &vulkan;
}

PulseBackendFlags VulkanCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_VULKAN) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_SPIRV_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	if(!VulkanInitLoader())
		return PULSE_BACKEND_INVALID;
	VulkanInstance instance;
	if(!VulkanInitInstance(&instance, PULSE_NO_DEBUG)) // Instance creation will fail if Vulkan is not supported
	{
		PulseGetLastErrorType(); // Clearing out the errors set by the failed instance creation
		return PULSE_BACKEND_INVALID;
	}
	VulkanDestroyInstance(&instance);
	VulkanLoaderShutdown();
	return PULSE_BACKEND_VULKAN;
}

bool VulkanLoadBackend(PulseDebugLevel debug_level)
{
	if(!VulkanInitLoader())
		return false;
	VulkanDriverData* driver_data = (VulkanDriverData*)calloc(1, sizeof(VulkanDriverData));
	PULSE_CHECK_ALLOCATION_RETVAL(driver_data, false);
	if(!VulkanInitInstance(&driver_data->instance, debug_level))
		return false;
	VulkanDriver.driver_data = driver_data;
	return true;
}

void VulkanUnloadBackend(PulseBackend backend)
{
	VulkanDestroyInstance(&VULKAN_RETRIEVE_DRIVER_DATA_AS(backend, VulkanDriverData*)->instance);
	VulkanLoaderShutdown();
}

const char* VulkanVerbaliseResult(VkResult res)
{
	switch(res)
	{
		case VK_SUCCESS: return "Success";
		case VK_NOT_READY: return "A fence or query has not yet completed";
		case VK_TIMEOUT: return "A wait operation has not completed in the specified time";
		case VK_EVENT_SET: return "An event is signaled";
		case VK_EVENT_RESET: return "An event is unsignaled";
		case VK_INCOMPLETE: return "A return array was too small for the result";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "A host memory allocation has failed";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "A device memory allocation has failed";
		case VK_ERROR_INITIALIZATION_FAILED: return "Initialization of an object could not be completed for implementation-specific reasons";
		case VK_ERROR_DEVICE_LOST: return "The logical or physical device has been lost";
		case VK_ERROR_MEMORY_MAP_FAILED: return "Mapping of a memory object has failed";
		case VK_ERROR_LAYER_NOT_PRESENT: return "A requested layer is not present or could not be loaded";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "A requested extension is not supported";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "A requested feature is not supported";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
		case VK_ERROR_TOO_MANY_OBJECTS: return "Too many objects of the type have already been created";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "A requested format is not supported on this device";
		case VK_ERROR_SURFACE_LOST_KHR: return "A surface is no longer available";
		case VK_SUBOPTIMAL_KHR: return "A swapchain no longer matches the surface properties exactly, but can still be used";
		case VK_ERROR_OUT_OF_DATE_KHR: return "A surface has changed in such a way that it is no longer compatible with the swapchain";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "The display used by a swapchain does not use the same presentable image layout";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
		case VK_ERROR_VALIDATION_FAILED_EXT: return "A validation layer found an error";

		default: return "Unknown Vulkan error";
	}
	return "Unknown Vulkan error"; // Just to avoid warnings
}

PulseBackendHandler VulkanDriver = {
	.PFN_LoadBackend = VulkanLoadBackend,
	.PFN_UnloadBackend = VulkanUnloadBackend,
	.PFN_CreateDevice = VulkanCreateDevice,
	.backend = PULSE_BACKEND_VULKAN,
	.supported_shader_formats = PULSE_SHADER_FORMAT_SPIRV_BIT,
	.driver_data = PULSE_NULLPTR
};
