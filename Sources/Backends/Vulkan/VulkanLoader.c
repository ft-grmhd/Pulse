// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"

static PulseLibModule vulkan_lib_module = PULSE_NULL_LIB_MODULE;
static uint32_t loader_references_count = 0;

static bool VulkanLoadGlobalFunctions(void* context, PFN_vkVoidFunction (*load)(void*, const char*));
static bool VulkanLoadInstanceFunctions(VulkanInstance* instance, PFN_vkVoidFunction (*load)(void*, const char*));
static bool VulkanLoadDeviceFunctions(VulkanInstance* instance, VulkanDevice* device, PFN_vkVoidFunction (*load)(VulkanInstance*, void*, const char*));

static inline PFN_vkVoidFunction vkGetInstanceProcAddrStub(void* context, const char* name)
{
	PFN_vkVoidFunction fn = VulkanGetGlobal()->vkGetInstanceProcAddr((VkInstance)context, name);
	if(!fn)
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return fn;
}

static inline PFN_vkVoidFunction vkGetDeviceProcAddrStub(VulkanInstance* instance, void* context, const char* name)
{
	PFN_vkVoidFunction fn = instance->vkGetDeviceProcAddr((VkDevice)context, name);
	if(!fn)
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return fn;
}

bool VulkanInitLoader()
{
	#if defined(PULSE_PLAT_WINDOWS)
		const char* libnames[] = {
			"vulkan-1.dll"
		};
	#elif defined(PULSE_PLAT_MACOS)
		const char* libnames[] = {
			"libvulkan.dylib",
			"libvulkan.1.dylib",
			"libMoltenVK.dylib",
			"vulkan.framework/vulkan",
			"MoltenVK.framework/MoltenVK",
			"/usr/local/lib/libvulkan.dylib",
		};
	#else
		const char* libnames[] = {
			"libvulkan.so.1",
			"libvulkan.so"
		};
	#endif

	if(loader_references_count != 0)
		return true;

	for(size_t i = 0; i < sizeof(libnames) / sizeof(const char*); i++)
	{
		vulkan_lib_module = PulseLoadLibrary(libnames[i]);
		if(vulkan_lib_module)
		{
			VulkanGetGlobal()->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)PulseLoadSymbolFromLibModule(vulkan_lib_module, "vkGetInstanceProcAddr");
			if(VulkanGetGlobal()->vkGetInstanceProcAddr == PULSE_NULLPTR)
			{
				PulseUnloadLibrary(vulkan_lib_module);
				vulkan_lib_module = PULSE_NULLPTR;
			}
			else
				break;
		}
	}
	if(!vulkan_lib_module)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return false;
	}
	loader_references_count++;
	return VulkanLoadGlobalFunctions(PULSE_NULLPTR, vkGetInstanceProcAddrStub);
}

bool VulkanLoadInstance(VulkanInstance* instance)
{
	return VulkanLoadInstanceFunctions(instance, vkGetInstanceProcAddrStub);
}

bool VulkanLoadDevice(VulkanInstance* instance, VulkanDevice* device)
{
	return VulkanLoadDeviceFunctions(instance, device, vkGetDeviceProcAddrStub);
}

void VulkanLoaderShutdown()
{
	loader_references_count--;
	if(loader_references_count != 0)
		return;
	PulseUnloadLibrary(vulkan_lib_module);
	vulkan_lib_module = PULSE_NULL_LIB_MODULE;
}

static bool VulkanLoadGlobalFunctions(void* context, PFN_vkVoidFunction (*load)(void*, const char*))
{
	#define PULSE_VULKAN_GLOBAL_FUNCTION(func) \
		VulkanGetGlobal()->func = (PFN_##func)load(context, #func); \
		if(!VulkanGetGlobal()->func) \
			return false;
		#include "VulkanGlobalPrototypes.h"
	#undef PULSE_VULKAN_GLOBAL_FUNCTION
	return true;
}

static bool VulkanLoadInstanceFunctions(VulkanInstance* instance, PFN_vkVoidFunction (*load)(void*, const char*))
{
	#define PULSE_VULKAN_INSTANCE_FUNCTION(func) \
		instance->func = (PFN_##func)load(instance->instance, #func); \
		if(!instance->func) \
			return false;
		#include "VulkanInstancePrototypes.h"
	#undef PULSE_VULKAN_INSTANCE_FUNCTION
	return true;
}

static bool VulkanLoadDeviceFunctions(VulkanInstance* instance, VulkanDevice* device, PFN_vkVoidFunction (*load)(VulkanInstance*, void*, const char*))
{
	#define PULSE_VULKAN_DEVICE_FUNCTION(func) \
		device->func = (PFN_##func)load(instance, device->device, #func); \
		if(!device->func) \
			return false;
		#include "VulkanDevicePrototypes.h"
	#undef PULSE_VULKAN_DEVICE_FUNCTION
	return true;
}
