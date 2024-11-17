// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_COMMAND_LIST_H_
#define PULSE_VULKAN_COMMAND_LIST_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "VulkanCommandPool.h"

typedef struct VulkanCommandList
{
	VulkanCommandPool* pool;
	VkCommandBuffer cmd;
} VulkanCommandList;

PulseCommandList VulkanRequestCommandList(PulseDevice device, PulseCommandListUsage usage);
bool VulkanSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
void VulkanReleaseCommandList(PulseDevice device, PulseCommandList cmd);

#endif // PULSE_VULKAN_COMMAND_LIST_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
