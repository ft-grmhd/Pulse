// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_COMPUTE_PIPELINE_H_
#define PULSE_VULKAN_COMPUTE_PIPELINE_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>

typedef struct VulkanComputePipeline
{
	VkShaderModule module;
	VkPipelineLayout layout;
	VkPipeline pipeline;
} VulkanComputePipeline;

PulseComputePipeline VulkanCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info);
void VulkanDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline);
void VulkanBindStorageBuffers(PulseComputePass pass, uint32_t starting_slot, PulseBuffer* const* buffers, uint32_t num_buffers);
void VulkanBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size);
void VulkanBindStorageImages(PulseComputePass pass, uint32_t starting_slot, PulseImage* const* images, uint32_t num_images);
void VulkanBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline);
void VulkanDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);

#endif // PULSE_VULKAN_COMPUTE_PIPELINE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
