// Copyright (C) 2024 kanel
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
void VulkanDispatchComputePipeline(PulseComputePipeline pipeline, PulseCommandList cmd, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);
void VulkanDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline);

#endif // PULSE_VULKAN_COMPUTE_PIPELINE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
