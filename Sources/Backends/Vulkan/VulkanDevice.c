// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "../../PulseInternal.h"
/*
static int32_t VulkanScorePhysicalDevice(VkPhysicalDevice device, const char** device_extensions, uint32_t device_extensions_count)
{
	PULSE_DECLARE_STACK_FIXED_ALLOCATOR(allocator, sizeof(VkExtensionProperties) * 4096, sizeof(VkExtensionProperties));
	// Check extensions support
	uint32_t extension_count;
	kbhGetVulkanPFNs()->vkEnumerateDeviceExtensionProperties(device, PULSE_NULLPTR, &extension_count, PULSE_NULLPTR);
	VkExtensionProperties* props = (VkExtensionProperties*)kbhCallocInFixed(&allocator, extension_count, sizeof(VkExtensionProperties));
	if(!props)
		return -1;
	kbhGetVulkanPFNs()->vkEnumerateDeviceExtensionProperties(device, PULSE_NULLPTR, &extension_count, props);

	bool are_there_required_device_extensions = true;
	for(int j = 0; j < device_extensions_count; j++)
	{
		bool is_there_extension = false;
		for(int k = 0; k < extension_count; k++)
		{
			if(strcmp(device_extensions[j], props[k].extensionName) == 0)
			{
				is_there_extension = true;
				break;
			}
		}
		if(is_there_extension == false)
		{
			are_there_required_device_extensions = false;
			break;
		}
	}
	if(are_there_required_device_extensions == false)
		return -1;

	// Check Queue Families Support
	int32_t queue;
	if(kbhFindPhysicalDeviceQueueFamily(device, KBH_VULKAN_QUEUE_COMPUTE, &queue) != KBH_RHI_SUCCESS)
		return -1;

	VkPhysicalDeviceProperties device_props;
	kbhGetVulkanPFNs()->vkGetPhysicalDeviceProperties(device, &device_props);

	VkPhysicalDeviceFeatures device_features;
	kbhGetVulkanPFNs()->vkGetPhysicalDeviceFeatures(device, &device_features);

	int32_t score = -1;
	if(device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1001;

	score += device_props.limits.maxComputeWorkGroupCount[0];
	score += device_props.limits.maxComputeWorkGroupCount[1];
	score += device_props.limits.maxComputeWorkGroupCount[2];
	score += device_props.limits.maxComputeSharedMemorySize;

	return score;
}

static VkPhysicalDevice VulkanPickPhysicalDevice(VulkanInstance* instance)
{
	VkPhysicalDevice* devices = PULSE_NULLPTR;
	VkPhysicalDevice chosen_one = VK_NULL_HANDLE;
	uint32_t device_count;
	int32_t best_device_score = -1;

	instance->vkEnumeratePhysicalDevices(instance->instance, &device_count, PULSE_NULLPTR);
	devices = (VkPhysicalDevice*)calloc(device_count, sizeof(VkPhysicalDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(devices, VK_NULL_HANDLE);
	instance->vkEnumeratePhysicalDevices(instance->instance, &device_count, devices);

	for(int i = 0; i < device_count; i++)
	{
		int32_t current_device_score = VulkanScorePhysicalDevice(devices[i], PULSE_NULLPTR, 0);
		if(current_device_score > best_device_score)
		{
			best_device_score = current_device_score;
			chosen_one = devices[i];
		}
	}
	return chosen_one;
}
*/

void* VulkanCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
}

void VulkanDestroyDevice(VulkanDevice* device)
{
	if(device == PULSE_NULLPTR || device->device == VK_NULL_HANDLE)
		return;
	vmaDestroyAllocator(device->allocator);
	device->vkDestroyDevice(device->device, PULSE_NULLPTR);
	device->device = VK_NULL_HANDLE;
}
