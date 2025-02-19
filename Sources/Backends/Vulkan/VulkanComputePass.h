// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_COMPUTE_PASS_H_
#define PULSE_VULKAN_COMPUTE_PASS_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "VulkanBuffer.h"
#include "VulkanDescriptor.h"
#include "VulkanCommandList.h"

typedef struct VulkanComputePass
{
	VulkanDescriptorSet* read_only_descriptor_set;
	VulkanDescriptorSet* read_write_descriptor_set;
	VulkanDescriptorSet* uniform_descriptor_set;
	PulseBuffer uniform_buffer;

	bool should_recreate_read_only_descriptor_sets;
	bool should_recreate_write_descriptor_sets;
	bool should_recreate_uniform_descriptor_sets;
} VulkanComputePass;

PulseComputePass VulkanCreateComputePass(PulseDevice device, PulseCommandList cmd);
void VulkanDestroyComputePass(PulseDevice device, PulseComputePass pass);

PulseComputePass VulkanBeginComputePass(PulseCommandList cmd);
void VulkanEndComputePass(PulseComputePass pass);
void VulkanBindStorageBuffers(PulseComputePass pass, uint32_t starting_slot, const PulseBuffer* buffers, uint32_t num_buffers);
void VulkanBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size);
void VulkanBindStorageImages(PulseComputePass pass, uint32_t starting_slot, const PulseImage* images, uint32_t num_images);
void VulkanBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline);
void VulkanDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);

#endif // PULSE_VULKAN_COMMAND_LIST_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
