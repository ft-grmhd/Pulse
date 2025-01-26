// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_COMMAND_LIST_H_
#define PULSE_VULKAN_COMMAND_LIST_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDescriptor.h"

typedef struct VulkanCommandList
{
	VulkanCommandPool* pool;
	VkCommandBuffer cmd;
} VulkanCommandList;

typedef struct VulkanComputePass
{
	VulkanDescriptorSet* read_only_descriptor_set;
	VulkanDescriptorSet* read_write_descriptor_set;
	VulkanDescriptorSet* uniform_descriptor_set;
	PulseBuffer uniform_buffer;
} VulkanComputePass;

PulseCommandList VulkanRequestCommandList(PulseDevice device, PulseCommandListUsage usage);
bool VulkanSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
void VulkanReleaseCommandList(PulseDevice device, PulseCommandList cmd);
PulseComputePass VulkanBeginComputePass(PulseCommandList cmd);
void VulkanEndComputePass(PulseComputePass pass);

#endif // PULSE_VULKAN_COMMAND_LIST_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
