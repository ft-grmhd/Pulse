// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include "Pulse.h"
#include "PulseProfile.h"
#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanDescriptor.h"

void VulkanInitDescriptorSetPool(VulkanDescriptorSetPool* pool, PulseDevice device)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	memset(pool, 0, sizeof(VulkanDescriptorSetPool));
	pool->device = device;

	VkDescriptorPoolSize pool_sizes[
		PULSE_MAX_STORAGE_TEXTURES_BOUND +
		PULSE_MAX_STORAGE_BUFFERS_BOUND +
		PULSE_MAX_WRITE_TEXTURES_BOUND +
		PULSE_MAX_WRITE_BUFFERS_BOUND +
		PULSE_MAX_UNIFORM_BUFFERS_BOUND];

	uint32_t i = 0;

	for(uint32_t start = i; i < start + PULSE_MAX_STORAGE_TEXTURES_BOUND; i++)
	{
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	for(uint32_t start = i; i < start + PULSE_MAX_STORAGE_BUFFERS_BOUND; i++)
	{
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	for(uint32_t start = i; i < start + PULSE_MAX_WRITE_TEXTURES_BOUND; i++)
	{
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	for(uint32_t start = i; i < start + PULSE_MAX_WRITE_BUFFERS_BOUND; i++)
	{
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	for(uint32_t start = i; i < start + PULSE_MAX_UNIFORM_BUFFERS_BOUND; i++)
	{
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	VkDescriptorPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
	pool_info.pPoolSizes = pool_sizes;
	pool_info.maxSets = VULKAN_POOL_SIZE;
	pool_info.flags = 0;
	CHECK_VK(device->backend, vulkan_device->vkCreateDescriptorPool(vulkan_device->device, &pool_info, PULSE_NULLPTR, &pool->pool), PULSE_ERROR_INITIALIZATION_FAILED);
}

VulkanDescriptorSet* VulkanRequestDescriptorSetFromPool(VulkanDescriptorSetPool* pool, const VulkanDescriptorSetLayout* layout)
{
	for(size_t i = 0; i < VULKAN_POOL_SIZE; i++)
	{
		if(pool->free_sets[i] == PULSE_NULLPTR)
			break; // Due to defragmentation we are sure not to find valid sets after the first NULL one
		if(pool->free_sets[i]->layout == layout)
		{
			VulkanDescriptorSet* set = pool->free_sets[i];
			PULSE_DEFRAG_ARRAY(pool->free_sets, VULKAN_POOL_SIZE, i);
			pool->free_index--;
			pool->used_sets[pool->used_index] = (VulkanDescriptorSet*)set;
			pool->used_index++;
			return set;
		}
	}

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pool->device, VulkanDevice*);

	VulkanDescriptorSet* set = (VulkanDescriptorSet*)calloc(1, sizeof(VulkanDescriptorSet));
	PULSE_CHECK_ALLOCATION_RETVAL(set, PULSE_NULLPTR);

	set->device = pool->device;
	set->pool = pool;
	set->layout = (VulkanDescriptorSetLayout*)layout;

	VkDescriptorSetAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = pool->pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &layout->layout;
	CHECK_VK_RETVAL(pool->device->backend, vulkan_device->vkAllocateDescriptorSets(vulkan_device->device, &alloc_info, &set->set), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULLPTR);

	pool->allocations_count++;
	return set;
}

void VulkanReturnDescriptorSetToPool(VulkanDescriptorSetPool* pool, const VulkanDescriptorSet* set)
{
	for(size_t i = 0; i < VULKAN_POOL_SIZE; i++)
	{
		if(pool->used_sets[i] == PULSE_NULLPTR)
			break; // Due to defragmentation we are sure not to find valid sets after the first NULL one
		if(pool->used_sets[i] == set)
		{
			PULSE_DEFRAG_ARRAY(pool->used_sets, VULKAN_POOL_SIZE, i);
			pool->used_index--;
			pool->free_sets[pool->free_index] = (VulkanDescriptorSet*)set;
			pool->free_index++;
			return;
		}
	}
}

void VulkanDestroyDescriptorSetPool(VulkanDescriptorSetPool* pool)
{
	if(pool->pool == VK_NULL_HANDLE)
		return;

	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pool->device, VulkanDevice*);

	for(size_t i = 0; i < VULKAN_POOL_SIZE; i++)
	{
		if(pool->used_sets[i] != PULSE_NULLPTR)
		{
			vulkan_device->vkDestroyDescriptorSetLayout(vulkan_device->device, pool->used_sets[i]->layout->layout, PULSE_NULLPTR);
			free(pool->used_sets[i]);
			pool->used_sets[i] = PULSE_NULLPTR;
		}
		if(pool->free_sets[i] != PULSE_NULLPTR)
		{
			vulkan_device->vkDestroyDescriptorSetLayout(vulkan_device->device, pool->free_sets[i]->layout->layout, PULSE_NULLPTR);
			free(pool->free_sets[i]);
			pool->free_sets[i] = PULSE_NULLPTR;
		}
		if(pool->free_sets[i] == PULSE_NULLPTR && pool->used_sets[i] == PULSE_NULLPTR)
			break; // Due to defragmentation we are sure not to find valid sets after the first NULL one
	}
	vulkan_device->vkDestroyDescriptorPool(vulkan_device->device, pool->pool, PULSE_NULLPTR);
	memset(pool, 0, sizeof(VulkanDescriptorSetPool));
}


void VulkanInitDescriptorSetPoolManager(VulkanDescriptorSetPoolManager* manager, PulseDevice device)
{
	memset(manager, 0, sizeof(VulkanDescriptorSetPoolManager));
	manager->device = device;
}

VulkanDescriptorSetPool* VulkanGetAvailableDescriptorSetPool(VulkanDescriptorSetPoolManager* manager)
{
	for(uint32_t i = 0; i < manager->pools_size; i++)
	{
		if(manager->pools[i]->allocations_count < VULKAN_POOL_SIZE || manager->pools[i]->free_sets[0] != PULSE_NULLPTR)
			return manager->pools[i];
	}
	PULSE_EXPAND_ARRAY_IF_NEEDED(manager->pools, VulkanDescriptorSetPool*, manager->pools_size, manager->pools_capacity, 1);
	PULSE_CHECK_ALLOCATION_RETVAL(manager->pools, PULSE_NULLPTR);

	manager->pools[manager->pools_size] = (VulkanDescriptorSetPool*)calloc(1, sizeof(VulkanDescriptorSetPool));
	PULSE_CHECK_ALLOCATION_RETVAL(manager->pools[manager->pools_size], PULSE_NULLPTR);

	VulkanInitDescriptorSetPool(manager->pools[manager->pools_size], manager->device);
	manager->pools_size++;
	return manager->pools[manager->pools_size - 1];
}

void VulkanDestroyDescriptorSetPoolManager(VulkanDescriptorSetPoolManager* manager)
{
	for(uint32_t i = 0; i < manager->pools_size; i++)
	{
		VulkanDestroyDescriptorSetPool(manager->pools[i]);
		free(manager->pools[i]);
	}
	free(manager->pools);
	memset(manager, 0, sizeof(VulkanDescriptorSetPoolManager));
}
