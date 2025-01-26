// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "../../PulseInternal.h"
#include "VulkanInstance.h"
#include "Pulse.h"
#include "PulseProfile.h"
#include "VulkanLoader.h"
#include "Vulkan.h"

#include <stdio.h>
#include <string.h>

static const char* layer_names[] = { "VK_LAYER_KHRONOS_validation" };

static bool CheckValidationLayerSupport()
{
	uint32_t layer_count;
	VulkanGetGlobal()->vkEnumerateInstanceLayerProperties(&layer_count, PULSE_NULLPTR);
	VkLayerProperties* available_layers = (VkLayerProperties*)calloc(layer_count, sizeof(VkLayerProperties));
	PULSE_CHECK_ALLOCATION_RETVAL(available_layers, false);
	VulkanGetGlobal()->vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

	for(size_t i = 0; i < sizeof(layer_names) / sizeof(layer_names[0]); i++)
	{
		bool found = false;
		for(size_t j = 0; j < layer_count; j++)
		{
			if(strcmp(available_layers[j].layerName, layer_names[i]) == 0)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			free(available_layers);
			return false;
		}
	}
	free(available_layers);
	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
	(void)type;
	PulseBackend backend = user_data;
	if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT || severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		PulseLogErrorFmt(backend, "(Vulkan) validation: %s", callback_data->pMessage);
	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, VkDebugUtilsMessengerEXT* messenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT)VulkanGetGlobal()->vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if(fn)
		return fn(instance, create_info, NULL, messenger);
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static bool InitValidationLayers(PulseBackend backend, VulkanInstance* instance)
{
	uint32_t extension_count;
	VulkanGetGlobal()->vkEnumerateInstanceExtensionProperties(PULSE_NULLPTR, &extension_count, NULL);
	VkExtensionProperties* extensions = (VkExtensionProperties*)calloc(extension_count, sizeof(VkExtensionProperties));
	PULSE_CHECK_ALLOCATION_RETVAL(extensions, false);
	VulkanGetGlobal()->vkEnumerateInstanceExtensionProperties(PULSE_NULLPTR, &extension_count, extensions);
	bool extension_found = false;
	for(uint32_t i = 0; i < extension_count; i++)
	{
		if(strcmp(extensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
		{
			extension_found = true;
			break;
		}
	}
	if(!extension_found)
	{
		fprintf(stderr, "Pulse: (Vulkan) " VK_EXT_DEBUG_UTILS_EXTENSION_NAME " is not present; cannot enable validation layers");
		free(extensions);
		return false;
	}

	VkDebugUtilsMessengerCreateInfoEXT create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = DebugCallback;
	create_info.pUserData = backend;
	CHECK_VK_RETVAL(backend, CreateDebugUtilsMessengerEXT(instance->instance, &create_info, &instance->debug_messenger), PULSE_ERROR_INITIALIZATION_FAILED, false);
	return true;
}

static void DestroyDebugUtilsMessengerEXT(VulkanInstance* instance)
{
	PFN_vkDestroyDebugUtilsMessengerEXT fn = (PFN_vkDestroyDebugUtilsMessengerEXT)VulkanGetGlobal()->vkGetInstanceProcAddr(instance->instance, "vkDestroyDebugUtilsMessengerEXT");
	if(fn)
		fn(instance->instance, instance->debug_messenger, PULSE_NULLPTR);
}

static VkInstance VulkanCreateInstance(PulseBackend backend, const char** extensions_enabled, uint32_t extensions_count, bool enable_validation_layers)
{
	VkInstance instance = VK_NULL_HANDLE;

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pEngineName = "PulseGPU";
	app_info.engineVersion = PULSE_VERSION;
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = extensions_count;
	create_info.ppEnabledExtensionNames = extensions_enabled;
	#ifdef PULSE_PLAT_MACOS
		create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	#else
		create_info.flags = 0;
	#endif

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = { 0 };
	const char** new_extension_set = PULSE_NULLPTR;
	if(enable_validation_layers)
	{
		new_extension_set = (const char**)calloc(extensions_count + 1, sizeof(char*));
		PULSE_CHECK_ALLOCATION_RETVAL(new_extension_set, PULSE_NULL_HANDLE);
		memcpy(new_extension_set, extensions_enabled, sizeof(char*) * extensions_count);
		new_extension_set[extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

		debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_create_info.pfnUserCallback = DebugCallback;
		debug_create_info.pUserData = backend;

		create_info.enabledExtensionCount = extensions_count + 1;
		create_info.ppEnabledExtensionNames = new_extension_set;
		create_info.enabledLayerCount = sizeof(layer_names) / sizeof(layer_names[0]);
		create_info.ppEnabledLayerNames = (const char* const*)layer_names;
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}

	CHECK_VK_RETVAL(PULSE_NULL_HANDLE, VulkanGetGlobal()->vkCreateInstance(&create_info, PULSE_NULLPTR, &instance), PULSE_ERROR_INITIALIZATION_FAILED, VK_NULL_HANDLE);
	if(enable_validation_layers)
		free(new_extension_set);
	return instance;
}

bool VulkanInitInstance(PulseBackend backend, VulkanInstance* instance, PulseDebugLevel debug_level)
{
	#ifdef PULSE_PLAT_MACOS
		const char* extensions[] = {
			VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
		};
	#else
		const char* extensions[] = {
		};
	#endif
	instance->validation_layers_enabled = (backend != PULSE_NULL_HANDLE && debug_level == PULSE_HIGH_DEBUG && CheckValidationLayerSupport());
	instance->instance = VulkanCreateInstance(backend, extensions, sizeof(extensions) / sizeof(char*), instance->validation_layers_enabled);
	if(instance->instance == VK_NULL_HANDLE)
		return false;
	if(!VulkanLoadInstance(instance))
		return false;
	if(instance->validation_layers_enabled)
		InitValidationLayers(backend, instance);
	return true;
}

void VulkanDestroyInstance(VulkanInstance* instance)
{
	if(instance == PULSE_NULLPTR || instance->instance == VK_NULL_HANDLE)
		return;
	if(instance->validation_layers_enabled)
		DestroyDebugUtilsMessengerEXT(instance);
	instance->vkDestroyInstance(instance->instance, PULSE_NULLPTR);
	instance->instance = VK_NULL_HANDLE;
}
