// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_DESCRIPTOR_H_
#define PULSE_VULKAN_DESCRIPTOR_H_

#include <vulkan/vulkan_core.h>

#include <Pulse.h>

#include "VulkanEnums.h"

#define VULKAN_POOL_SIZE 128

typedef struct VulkanDescriptorSetLayout
{
	VkDescriptorSetLayout layout;
	VulkanDescriptorSetType type;

	union
	{
		struct
		{
			uint32_t storage_buffer_count;
			uint32_t storage_texture_count;
		} ReadOnly;
		struct
		{
			uint32_t storage_buffer_count;
			uint32_t storage_texture_count;
		} ReadWrite;
		struct
		{
			uint32_t buffer_count;
		} Uniform;
	};
} VulkanDescriptorSetLayout;

typedef struct VulkanDescriptorSet
{
	PulseDevice device;
	VulkanDescriptorSetLayout* layout;
	struct VulkanDescriptorSetPool* pool;
	VkDescriptorSet set;
} VulkanDescriptorSet;

typedef struct VulkanDescriptorSetPool
{
	VulkanDescriptorSet* used_sets[VULKAN_POOL_SIZE];
	VulkanDescriptorSet* free_sets[VULKAN_POOL_SIZE];
	uint32_t used_index;
	uint32_t free_index;
	PulseDevice device;
	VkDescriptorPool pool;
	uint32_t allocations_count;
} VulkanDescriptorSetPool;

typedef struct VulkanDescriptorSetPoolManager
{
	PulseDevice device;
	VulkanDescriptorSetPool** pools;
	uint32_t pools_capacity;
	uint32_t pools_size;
} VulkanDescriptorSetPoolManager;

void VulkanInitDescriptorSetPool(VulkanDescriptorSetPool* pool, PulseDevice device);
VulkanDescriptorSet* VulkanRequestDescriptorSetFromPool(VulkanDescriptorSetPool* pool, const VulkanDescriptorSetLayout* layout);
void VulkanReturnDescriptorSetToPool(VulkanDescriptorSetPool* pool, const VulkanDescriptorSet* set);
void VulkanDestroyDescriptorSetPool(VulkanDescriptorSetPool* pool);

void VulkanInitDescriptorSetPoolManager(VulkanDescriptorSetPoolManager* manager, PulseDevice device);
VulkanDescriptorSetPool* VulkanGetAvailableDescriptorSetPool(VulkanDescriptorSetPoolManager* manager);
void VulkanDestroyDescriptorSetPoolManager(VulkanDescriptorSetPoolManager* manager);

#endif // PULSE_VULKAN_DESCRIPTOR_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
