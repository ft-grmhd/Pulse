// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_BUFFER_H_
#define PULSE_VULKAN_BUFFER_H_

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "VulkanEnums.h"

typedef struct VulkanBuffer
{
	VkBuffer buffer;
	VkBufferUsageFlags usage;
	VmaAllocation allocation;
} VulkanBuffer;

PulseBuffer VulkanCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos);
void VulkanDestroyBuffer(PulseDevice device, PulseBuffer buffer);

#endif // PULSE_VULKAN_BUFFER_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
