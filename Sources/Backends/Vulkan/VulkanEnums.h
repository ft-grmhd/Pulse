// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_ENUMS_H_
#define PULSE_VULKAN_ENUMS_H_

typedef enum VulkanQueueType
{
	VULKAN_QUEUE_COMPUTE = 0,
	VULKAN_QUEUE_TRANSFER,

	VULKAN_QUEUE_END_ENUM // For internal use only
} VulkanQueueType;

typedef enum VulkanDescriptorSetType
{
	VULKAN_DESCRIPTOR_SET_READ_ONLY = 0,
	VULKAN_DESCRIPTOR_SET_READ_WRITE,
	VULKAN_DESCRIPTOR_SET_UNIFORM,

	VULKAN_DESCRIPTOR_SET_END_ENUM // For internal use only
} VulkanDescriptorSetType;

#endif // PULSE_VULKAN_ENUMS_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
