// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "Vulkan.h"
#include "VulkanComputePipeline.h"
#include "VulkanCommandList.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanInstance.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanComputePass.h"
#include "../../PulseInternal.h"

#include <string.h>

static int32_t VulkanScorePhysicalDevice(VulkanInstance* instance, VkPhysicalDevice device, const char** device_extensions, uint32_t device_extensions_count)
{
	// Check extensions support
	uint32_t extension_count;
	instance->vkEnumerateDeviceExtensionProperties(device, PULSE_NULLPTR, &extension_count, PULSE_NULLPTR);
	VkExtensionProperties* props = (VkExtensionProperties*)calloc(extension_count, sizeof(VkExtensionProperties));
	if(!props)
		return -1;
	instance->vkEnumerateDeviceExtensionProperties(device, PULSE_NULLPTR, &extension_count, props);

	bool are_there_required_device_extensions = true;
	for(uint32_t j = 0; j < device_extensions_count; j++)
	{
		bool is_there_extension = false;
		for(uint32_t k = 0; k < extension_count; k++)
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
	if(!VulkanFindPhysicalDeviceQueueFamily(instance, device, VULKAN_QUEUE_COMPUTE, &queue))
		return -1;

	VkPhysicalDeviceProperties device_props;
	instance->vkGetPhysicalDeviceProperties(device, &device_props);

	VkPhysicalDeviceFeatures device_features;
	instance->vkGetPhysicalDeviceFeatures(device, &device_features);

	int32_t score = -1;
	if(device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 10000;

	score += device_props.limits.maxComputeWorkGroupCount[0];
	score += device_props.limits.maxComputeWorkGroupCount[1];
	score += device_props.limits.maxComputeWorkGroupCount[2];
	score += device_props.limits.maxComputeSharedMemorySize;

	return score;
}

static bool VulkanIsDeviceForbidden(VkPhysicalDevice device, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	if(device == VK_NULL_HANDLE)
		return true;
	#pragma omp parallel for
	for(uint32_t i = 0; i < forbiden_devices_count; i++)
	{
		if(device == ((VulkanDevice*)forbiden_devices[i]->driver_data)->physical)
			return true;
	}
	return false;
}

static VkPhysicalDevice VulkanPickPhysicalDevice(VulkanInstance* instance, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	VkPhysicalDevice* devices = PULSE_NULLPTR;
	VkPhysicalDevice chosen_one = VK_NULL_HANDLE;
	uint32_t device_count;
	int32_t best_device_score = -1;

	instance->vkEnumeratePhysicalDevices(instance->instance, &device_count, PULSE_NULLPTR);
	devices = (VkPhysicalDevice*)calloc(device_count, sizeof(VkPhysicalDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(devices, VK_NULL_HANDLE);
	instance->vkEnumeratePhysicalDevices(instance->instance, &device_count, devices);

	#pragma omp parallel for
	for(uint32_t i = 0; i < device_count; i++)
	{
		if(VulkanIsDeviceForbidden(devices[i], forbiden_devices, forbiden_devices_count))
			continue;
		int32_t current_device_score = VulkanScorePhysicalDevice(instance, devices[i], PULSE_NULLPTR, 0);
		if(current_device_score > best_device_score)
		{
			best_device_score = current_device_score;
			chosen_one = devices[i];
		}
	}
	return chosen_one;
}

PulseDevice VulkanCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULLPTR);

	PulseDevice pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);

	VulkanDevice* device = (VulkanDevice*)calloc(1, sizeof(VulkanDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(device, PULSE_NULLPTR);

	VulkanInstance* instance = &VULKAN_RETRIEVE_DRIVER_DATA_AS(backend, VulkanDriverData*)->instance;

	device->physical = VulkanPickPhysicalDevice(instance, forbiden_devices, forbiden_devices_count);
	PULSE_CHECK_HANDLE_RETVAL(device->physical, PULSE_NULLPTR);

	const float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo* queue_create_infos = (VkDeviceQueueCreateInfo*)PulseStaticAllocStack(VULKAN_QUEUE_END_ENUM * sizeof(VkDeviceQueueCreateInfo));
	// No need to check allocation, it is allocated on the stack

	uint32_t unique_queues_count = 1;

	for(int32_t i = 0; i < VULKAN_QUEUE_END_ENUM; i++) // Needs to be done before next loop
	{
		if(!VulkanPrepareDeviceQueue(instance, device, i))
		{
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			return PULSE_NULLPTR;
		}
	}

	for(int32_t i = 0; i < VULKAN_QUEUE_END_ENUM; i++)
	{
		if(device->queues[i]->queue_family_index == -1)
			continue;
		int j;
		for(j = i; j < VULKAN_QUEUE_END_ENUM; j++) // Ugly shit but array will never be big so it's okay
		{
			if(device->queues[i]->queue_family_index == device->queues[j]->queue_family_index)
				break;
		}
		if(j == VULKAN_QUEUE_END_ENUM)
			unique_queues_count++;
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = device->queues[i]->queue_family_index;
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = &queue_priority;
		queue_create_infos[i].flags = 0;
		queue_create_infos[i].pNext = PULSE_NULLPTR;
	}

	instance->vkGetPhysicalDeviceProperties(device->physical, &device->properties);
	instance->vkGetPhysicalDeviceMemoryProperties(device->physical, &device->memory_properties);
	instance->vkGetPhysicalDeviceFeatures(device->physical, &device->features);

	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = unique_queues_count;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.pEnabledFeatures = &device->features;
	create_info.enabledExtensionCount = 0;
	create_info.ppEnabledExtensionNames = PULSE_NULLPTR;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = PULSE_NULLPTR;
	create_info.flags = 0;
	create_info.pNext = PULSE_NULLPTR;

	CHECK_VK_RETVAL(backend, instance->vkCreateDevice(device->physical, &create_info, PULSE_NULLPTR, &device->device), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULLPTR);
	if(!VulkanLoadDevice(instance, device))
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogInfoFmt(backend, "(Vulkan) Could not load device functions from %s", device->properties.deviceName);
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return PULSE_NULLPTR;
	}

	for(int32_t i = 0; i < VULKAN_QUEUE_END_ENUM; i++)
	{
		if(!VulkanRetrieveDeviceQueue(device, (VulkanQueueType)i))
		{
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			return PULSE_NULLPTR;
		}
	}

	VmaVulkanFunctions vma_vulkan_func = {};
	vma_vulkan_func.vkAllocateMemory                    = device->vkAllocateMemory;
	vma_vulkan_func.vkBindBufferMemory                  = device->vkBindBufferMemory;
	vma_vulkan_func.vkBindImageMemory                   = device->vkBindImageMemory;
	vma_vulkan_func.vkCreateBuffer                      = device->vkCreateBuffer;
	vma_vulkan_func.vkCreateImage                       = device->vkCreateImage;
	vma_vulkan_func.vkDestroyBuffer                     = device->vkDestroyBuffer;
	vma_vulkan_func.vkDestroyImage                      = device->vkDestroyImage;
	vma_vulkan_func.vkFlushMappedMemoryRanges           = device->vkFlushMappedMemoryRanges;
	vma_vulkan_func.vkFreeMemory                        = device->vkFreeMemory;
	vma_vulkan_func.vkGetBufferMemoryRequirements       = device->vkGetBufferMemoryRequirements;
	vma_vulkan_func.vkGetImageMemoryRequirements        = device->vkGetImageMemoryRequirements;
	vma_vulkan_func.vkInvalidateMappedMemoryRanges      = device->vkInvalidateMappedMemoryRanges;
	vma_vulkan_func.vkMapMemory                         = device->vkMapMemory;
	vma_vulkan_func.vkUnmapMemory                       = device->vkUnmapMemory;
	vma_vulkan_func.vkCmdCopyBuffer                     = device->vkCmdCopyBuffer;
	vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = instance->vkGetPhysicalDeviceMemoryProperties;
	vma_vulkan_func.vkGetPhysicalDeviceProperties       = instance->vkGetPhysicalDeviceProperties;

	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_0;
	allocator_create_info.physicalDevice = device->physical;
	allocator_create_info.device = device->device;
	allocator_create_info.instance = instance->instance;
	allocator_create_info.pVulkanFunctions = &vma_vulkan_func;

	CHECK_VK_RETVAL(backend, vmaCreateAllocator(&allocator_create_info, &device->allocator), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULLPTR);

	pulse_device->driver_data = device;
	pulse_device->backend = backend;
	PULSE_LOAD_DRIVER_DEVICE(Vulkan);

	VulkanInitDescriptorSetPoolManager(&device->descriptor_set_pool_manager, pulse_device);
	VulkanInitDescriptorSetLayoutManager(&device->descriptor_set_layout_manager, pulse_device);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
		PulseLogInfoFmt(backend, "(Vulkan) created device from %s", device->properties.deviceName);
	return pulse_device;
}

void VulkanDestroyDevice(PulseDevice device)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	if(vulkan_device == PULSE_NULLPTR || vulkan_device->device == VK_NULL_HANDLE)
		return;
	VulkanDestroyDescriptorSetPoolManager(&vulkan_device->descriptor_set_pool_manager);
	VulkanDestroyDescriptorSetLayoutManager(&vulkan_device->descriptor_set_layout_manager);
	for(uint32_t i = 0; i < vulkan_device->cmd_pools_size; i++)
		vulkan_device->vkDestroyCommandPool(vulkan_device->device, vulkan_device->cmd_pools[i].pool, PULSE_NULLPTR);
	vmaDestroyAllocator(vulkan_device->allocator);
	vulkan_device->vkDestroyDevice(vulkan_device->device, PULSE_NULLPTR);
	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Vulkan) destroyed device created from %s", vulkan_device->properties.deviceName);
	free(vulkan_device->cmd_pools);
	free(vulkan_device);
	free(device);
}

VulkanCommandPool* VulkanRequestCmdPoolFromDevice(PulseDevice device, VulkanQueueType queue_type)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULLPTR);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	if(vulkan_device == PULSE_NULLPTR || vulkan_device->device == VK_NULL_HANDLE)
		return PULSE_NULLPTR;

	PulseThreadID thread_id = PulseGetThreadID();

	for(uint32_t i = 0; i < vulkan_device->cmd_pools_size; i++)
	{
		if(thread_id == vulkan_device->cmd_pools[i].thread_id && queue_type == vulkan_device->cmd_pools[i].queue_type)
			return &vulkan_device->cmd_pools[i];
	}

	vulkan_device->cmd_pools_size++;
	vulkan_device->cmd_pools = (VulkanCommandPool*)realloc(vulkan_device->cmd_pools, vulkan_device->cmd_pools_size * sizeof(VulkanCommandPool));
	if(!VulkanInitCommandPool(device, &vulkan_device->cmd_pools[vulkan_device->cmd_pools_size - 1], queue_type))
		return PULSE_NULLPTR;
	return &vulkan_device->cmd_pools[vulkan_device->cmd_pools_size - 1];
}
