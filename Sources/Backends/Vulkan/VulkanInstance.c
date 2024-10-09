// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "../../PulseInternal.h"
#include "VulkanInstance.h"
#include "VulkanLoader.h"
#include "Vulkan.h"

static VkInstance VulkanCreateInstance(const char** extensions_enabled, uint32_t extensions_count, PulseDebugLevel debug_level)
{
	static const char* layer_names[] = { "VK_LAYER_KHRONOS_validation" };

	VkInstance instance = VK_NULL_HANDLE;

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pEngineName = "PulseGPU";
	app_info.engineVersion = PULSE_VERSION;
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = extensions_count;
	create_info.ppEnabledExtensionNames = extensions_enabled;
	if(debug_level == PULSE_NO_DEBUG)
		create_info.enabledLayerCount = 0;
	else
		create_info.enabledLayerCount = sizeof(layer_names) / sizeof(layer_names[0]);
	create_info.ppEnabledLayerNames = layer_names;
	create_info.pNext = PULSE_NULLPTR;
	#ifdef PULSE_PLAT_MACOS
		create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	#else
		create_info.flags = 0;
	#endif

	VulkanGetGlobal()->vkCreateInstance(&create_info, PULSE_NULLPTR, &instance);
	return instance;
}

bool VulkanInitInstance(VulkanInstance* instance, PulseDebugLevel debug_level)
{
	#ifdef PULSE_PLAT_MACOS
		const char* extensions[] = {
			VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
		};
	#else
		const char* extensions[] = {
		};
	#endif
	instance->instance = VulkanCreateInstance(extensions, sizeof(extensions) / sizeof(char*), debug_level);
	if(instance->instance == VK_NULL_HANDLE)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return false;
	}
	if(!VulkanLoadInstance(instance))
		return false;
	return true;
}

void VulkanDestroyInstance(VulkanInstance* instance)
{
	if(instance == PULSE_NULLPTR || instance->instance == VK_NULL_HANDLE)
		return;
	instance->vkDestroyInstance(instance->instance, PULSE_NULLPTR);
	instance->instance = VK_NULL_HANDLE;
}
