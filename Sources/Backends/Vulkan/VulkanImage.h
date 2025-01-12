// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_IMAGE_H_
#define PULSE_VULKAN_IMAGE_H_

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "VulkanEnums.h"

typedef struct VulkanImage
{
	VkImage image;
	VkImageView view;
	VkImageUsageFlags usage;
	VmaAllocation allocation;
	VmaAllocationInfo allocation_info;
} VulkanImage;

PulseImage VulkanCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos);
bool VulkanIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage);
bool VulkanCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst);
bool VulkanBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst);
void VulkanDestroyImage(PulseDevice device, PulseImage image);

#endif // PULSE_VULKAN_IMAGE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
