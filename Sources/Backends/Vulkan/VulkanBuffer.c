// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "Vulkan.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "VulkanCommandList.h"

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

	VmaAllocationCreateInfo allocation_create_info = { 0 };
	allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

	if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_UPLOAD)
	{
		vulkan_buffer->usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	}
	if(buffer->usage & PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD)
	{
		vulkan_buffer->usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	}
	if(buffer->usage & PULSE_BUFFER_USAGE_UNIFORM_ACCESS)
	{
		vulkan_buffer->usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	}
	if(buffer->usage & PULSE_BUFFER_USAGE_STORAGE_READ || buffer->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE)
		vulkan_buffer->usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	VkBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = buffer->size;
	buffer_create_info.usage = vulkan_buffer->usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	CHECK_VK_RETVAL(device->backend, vmaCreateBuffer(vulkan_device->allocator, &buffer_create_info, &allocation_create_info, &vulkan_buffer->buffer, &vulkan_buffer->allocation, PULSE_NULLPTR), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);
	vmaGetAllocationInfo(vulkan_device->allocator, vulkan_buffer->allocation, &vulkan_buffer->allocation_info);

	return buffer;
}

bool VulkanMapBuffer(PulseBuffer buffer, void** data)
{
	VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer, VulkanBuffer*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer->device, VulkanDevice*);
	CHECK_VK_RETVAL(buffer->device->backend, vmaMapMemory(vulkan_device->allocator, vulkan_buffer->allocation, data), PULSE_ERROR_MAP_FAILED, false);
	return true;
}

void VulkanUnmapBuffer(PulseBuffer buffer)
{
	VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer, VulkanBuffer*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer->device, VulkanDevice*);
	vmaUnmapMemory(vulkan_device->allocator, vulkan_buffer->allocation);
}

bool VulkanCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
	VulkanBuffer* vulkan_src_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(src->buffer, VulkanBuffer*);
	VulkanBuffer* vulkan_dst_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(dst->buffer, VulkanBuffer*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(src->buffer->device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(cmd, VulkanCommandList*);

	VkBufferCopy copy_region = {};
	copy_region.srcOffset = src->offset;
	copy_region.dstOffset = dst->offset;
	copy_region.size = (src->size < dst->size ? src->size : dst->size);
	vulkan_device->vkCmdCopyBuffer(vulkan_cmd->cmd, vulkan_src_buffer->buffer, vulkan_dst_buffer->buffer, 1, &copy_region);

	return true;
}

bool VulkanCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
	VulkanBuffer* vulkan_src_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(src->buffer, VulkanBuffer*);
	VulkanImage* vulkan_dst_image = VULKAN_RETRIEVE_DRIVER_DATA_AS(dst->image, VulkanImage*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(src->buffer->device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(cmd, VulkanCommandList*);

	VkBufferImageCopy region = { 0 };
	region.bufferOffset = src->offset;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = dst->layer;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = dst->x;
	region.imageOffset.y = dst->y;
	region.imageOffset.z = dst->z;
	region.imageExtent.width = dst->width;
	region.imageExtent.height = dst->height;
	region.imageExtent.depth = dst->depth;

	vulkan_device->vkCmdCopyBufferToImage(vulkan_cmd->cmd, vulkan_src_buffer->buffer, vulkan_dst_image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	return true;
}

void VulkanDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(buffer, VulkanBuffer*);
	vmaDestroyBuffer(vulkan_device->allocator, vulkan_buffer->buffer, vulkan_buffer->allocation);
	free(vulkan_buffer);
	free(buffer);
}
