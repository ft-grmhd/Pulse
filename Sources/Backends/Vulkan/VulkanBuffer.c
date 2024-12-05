// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "Vulkan.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"

PulseBuffer VulkanCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);

	PulseBufferHandler* buffer = (PulseBufferHandler*)calloc(1, sizeof(PulseBufferHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(buffer, PULSE_NULL_HANDLE);

	VulkanBuffer* vulkan_buffer = (VulkanBuffer*)calloc(1, sizeof(VulkanBuffer));
	PULSE_CHECK_ALLOCATION_RETVAL(vulkan_buffer, PULSE_NULL_HANDLE);

	buffer->device = device;
	buffer->driver_data = vulkan_buffer;
	buffer->size = create_infos->size;
	buffer->usage = create_infos->usage;

	if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_UPLOAD)
		vulkan_buffer->usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD)
		vulkan_buffer->usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if(buffer->usage & PULSE_BUFFER_USAGE_STORAGE_READ || buffer->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE)
		vulkan_buffer->usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if(buffer->usage & PULSE_BUFFER_USAGE_UNIFORM_ACCESS)
		vulkan_buffer->usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo allocation_create_info = { 0 };
	allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

	if(buffer->usage & PULSE_BUFFER_USAGE_HOST_ACCESS)
		allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VkBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = buffer->size;
	buffer_create_info.usage = vulkan_buffer->usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	CHECK_VK_RETVAL(device->backend, vmaCreateBuffer(vulkan_device->allocator, &buffer_create_info, &allocation_create_info, &vulkan_buffer->buffer, &vulkan_buffer->allocation, PULSE_NULLPTR), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);
	vmaGetAllocationInfo(vulkan_device->allocator, vulkan_buffer->allocation, &vulkan_buffer->allocation_info);

	return buffer;
}

bool VulkanGetBufferMap(PulseBuffer buffer, void** data)
{
	VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer, VulkanBuffer*);
	if(vulkan_buffer->allocation_info.pMappedData == PULSE_NULLPTR)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(buffer->device->backend))
			PulseLogErrorFmt(buffer->device->backend, "(Vulkan) cannot map a buffer (%p); Buffer has to be created with PULSE_BUFFER_USAGE_HOST_ACCESS flag", buffer);
		PulseSetInternalError(PULSE_ERROR_MAP_FAILED);
		return false;
	}
	*data = vulkan_buffer->allocation_info.pMappedData;
	return true;
}

void VulkanDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer, VulkanBuffer*);
	vmaDestroyBuffer(vulkan_device->allocator, vulkan_buffer->buffer, vulkan_buffer->allocation);
}
