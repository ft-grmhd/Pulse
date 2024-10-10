// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"

PulseFence VulkanCreateFence(PulseDevice device)
{
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkFence vulkan_fence;
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	CHECK_VK_RETVAL(vulkan_device->vkCreateFence(vulkan_device->device, &fence_info, PULSE_NULLPTR, &vulkan_fence), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);

	PulseFenceHandler* fence = (PulseFenceHandler*)malloc(sizeof(PulseFenceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(fence, PULSE_NULL_HANDLE);
	fence->driver_data = vulkan_fence;
	return fence;
}

void VulkanDestroyFence(PulseDevice device, PulseFence fence)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	if(vulkan_device == PULSE_NULLPTR || vulkan_device->device == VK_NULL_HANDLE)
		return;
	VkFence vulkan_fence = VULKAN_RETRIEVE_DRIVER_DATA_AS(fence, VkFence);
	if(vulkan_fence == VK_NULL_HANDLE)
		return;
	vulkan_device->vkDestroyFence(vulkan_device->device, vulkan_fence, PULSE_NULLPTR);
	free(fence);
}

bool VulkanIsFenceReady(PulseDevice device, PulseFence fence)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	if(vulkan_device == PULSE_NULLPTR || vulkan_device->device == VK_NULL_HANDLE)
		return false;
	VkFence vulkan_fence = VULKAN_RETRIEVE_DRIVER_DATA_AS(fence, VkFence);
	if(vulkan_fence == VK_NULL_HANDLE)
		return false;
	VkResult res = vulkan_device->vkGetFenceStatus(vulkan_device->device, vulkan_fence);
	switch(res)
	{
		case VK_ERROR_DEVICE_LOST: PulseSetInternalError(PULSE_ERROR_DEVICE_LOST); return false;
		case VK_NOT_READY: return false;
		case VK_SUCCESS: return true;

		default: return false;
	}
	return false;
}

bool VulkanWaitForFences(PulseDevice device, PulseFence *const *fences, uint32_t fences_count, bool wait_for_all)
{
}
