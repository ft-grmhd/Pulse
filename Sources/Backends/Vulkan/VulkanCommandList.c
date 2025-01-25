// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "Vulkan.h"
#include "VulkanCommandList.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"

static void VulkanInitCommandList(VulkanCommandPool* pool, PulseCommandList cmd)
{
	PULSE_CHECK_PTR(pool);
	PULSE_CHECK_PTR(cmd);

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pool->device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(cmd, VulkanCommandList*);

	vulkan_cmd->pool = pool;

	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = pool->pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;
	CHECK_VK(pool->device->backend, vulkan_device->vkAllocateCommandBuffers(vulkan_device->device, &info, &vulkan_cmd->cmd), PULSE_ERROR_INITIALIZATION_FAILED);

	PULSE_EXPAND_ARRAY_IF_NEEDED(pool->available_command_lists, PulseCommandList, pool->available_command_lists_size, pool->available_command_lists_capacity, 5);
	pool->available_command_lists[pool->available_command_lists_size] = cmd;
	pool->available_command_lists_size++;
}

PulseComputePass VulkanCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	VulkanComputePass* vulkan_pass = (VulkanComputePass*)calloc(1, sizeof(VulkanComputePass));
	PULSE_CHECK_ALLOCATION_RETVAL(vulkan_pass, PULSE_NULL_HANDLE);

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);

//	vulkan_pass->read_only_descriptor_set = VulkanRequestDescriptorSetFromPool(
//			VulkanGetAvailableDescriptorSetPool(&vulkan_device->descriptor_set_pool_manager),
//			VulkanGetDescriptorSetLayout(&vulkan_device->descriptor_set_layout_manager, ));

	pass->cmd = cmd;
	pass->driver_data = vulkan_pass;

	return pass;
}

PulseCommandList VulkanRequestCommandList(PulseDevice device, PulseCommandListUsage usage)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);

	VulkanCommandPool* pool;
	switch(usage)
	{
		case PULSE_COMMAND_LIST_TRANSFER_ONLY: pool = VulkanRequestCmdPoolFromDevice(device, VULKAN_QUEUE_TRANSFER); break;
		case PULSE_COMMAND_LIST_GENERAL: // fallthrough
		default: pool = VulkanRequestCmdPoolFromDevice(device, VULKAN_QUEUE_COMPUTE); break;
	}

	PULSE_CHECK_PTR_RETVAL(pool, PULSE_NULL_HANDLE);

	PulseCommandList cmd = PULSE_NULL_HANDLE;

	for(uint32_t i = 0; i < pool->available_command_lists_size; i++)
	{
		if(pool->available_command_lists[i]->is_available)
		{
			cmd = pool->available_command_lists[i];
			break;
		}
	}

	if(cmd == PULSE_NULL_HANDLE)
	{
		cmd = (PulseCommandList)calloc(1, sizeof(PulseCommandListHandler));
		PULSE_CHECK_ALLOCATION_RETVAL(cmd, PULSE_NULL_HANDLE);

		VulkanCommandList* vulkan_cmd = (VulkanCommandList*)calloc(1, sizeof(VulkanCommandList));
		PULSE_CHECK_ALLOCATION_RETVAL(vulkan_cmd, PULSE_NULL_HANDLE);

		cmd->usage = usage;
		cmd->device = device;
		cmd->driver_data = vulkan_cmd;
		cmd->thread_id = pool->thread_id;

		VulkanInitCommandList(pool, cmd);
	}

	cmd->pass = VulkanCreateComputePass(device, cmd);
	cmd->state = PULSE_COMMAND_LIST_STATE_RECORDING;
	cmd->is_available = false;

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(cmd, VulkanCommandList*);

	CHECK_VK_RETVAL(device->backend, vulkan_device->vkResetCommandBuffer(vulkan_cmd->cmd, 0), PULSE_ERROR_DEVICE_ALLOCATION_FAILED, PULSE_NULL_HANDLE);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	VkResult res = vulkan_device->vkBeginCommandBuffer(vulkan_cmd->cmd, &begin_info);
	switch(res)
	{
		case VK_SUCCESS: break;
		case VK_ERROR_OUT_OF_HOST_MEMORY: PulseSetInternalError(PULSE_ERROR_CPU_ALLOCATION_FAILED); return PULSE_NULL_HANDLE;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: PulseSetInternalError(PULSE_ERROR_DEVICE_ALLOCATION_FAILED); return PULSE_NULL_HANDLE;
		default: break;
	}
	return cmd;
}

bool VulkanSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence)
{
	PULSE_CHECK_HANDLE_RETVAL(cmd, false);

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(cmd, VulkanCommandList*);

	VkResult res = vulkan_device->vkEndCommandBuffer(vulkan_cmd->cmd);
	switch(res)
	{
		case VK_SUCCESS: break;
		case VK_ERROR_OUT_OF_HOST_MEMORY: PulseSetInternalError(PULSE_ERROR_CPU_ALLOCATION_FAILED); return false;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: PulseSetInternalError(PULSE_ERROR_DEVICE_ALLOCATION_FAILED); return false;
		default: break;
	}

	VkFence vulkan_fence;
	if(fence != PULSE_NULL_HANDLE)
	{
		vulkan_fence = VULKAN_RETRIEVE_DRIVER_DATA_AS(fence, VkFence);
		CHECK_VK_RETVAL(device->backend, vulkan_device->vkResetFences(vulkan_device->device, 1, &vulkan_fence), PULSE_ERROR_DEVICE_ALLOCATION_FAILED, false);
		fence->cmd = cmd;
	}

	VulkanQueue* vulkan_queue;
	switch(cmd->usage)
	{
		case PULSE_COMMAND_LIST_TRANSFER_ONLY: vulkan_queue = vulkan_device->queues[VULKAN_QUEUE_TRANSFER]; break;
		case PULSE_COMMAND_LIST_GENERAL: // fallthrough
		default: vulkan_queue = vulkan_device->queues[VULKAN_QUEUE_COMPUTE]; break;
	}

	PULSE_CHECK_PTR_RETVAL(vulkan_queue, false);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vulkan_cmd->cmd;
	res = vulkan_device->vkQueueSubmit(vulkan_queue->queue, 1, &submit_info, vulkan_fence);
	cmd->state = PULSE_COMMAND_LIST_STATE_SENT;
	switch(res)
	{
		case VK_SUCCESS: return true;

		case VK_ERROR_OUT_OF_HOST_MEMORY: PulseSetInternalError(PULSE_ERROR_CPU_ALLOCATION_FAILED); return false;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: PulseSetInternalError(PULSE_ERROR_DEVICE_ALLOCATION_FAILED); return false;
		case VK_ERROR_DEVICE_LOST: PulseSetInternalError(PULSE_ERROR_DEVICE_LOST); return false;

		default: return false;
	}
	return false;
}

void VulkanReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	PULSE_CHECK_HANDLE(device);

	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(cmd, VulkanCommandList*);

	for(uint32_t i = 0; i < vulkan_cmd->pool->available_command_lists_size; i++)
	{
		if(vulkan_cmd->pool->available_command_lists[i] == cmd)
		{
			cmd->is_available = true;
			cmd->state = PULSE_COMMAND_LIST_STATE_INVALID;
			break;
		}
	}
}

PulseComputePass VulkanBeginComputePass(PulseCommandList cmd)
{
	return cmd->pass;
}

void VulkanEndComputePass(PulseComputePass pass)
{
}
