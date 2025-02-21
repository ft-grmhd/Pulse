// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include "Pulse.h"
#include "PulseProfile.h"
#include "Vulkan.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "VulkanDescriptor.h"
#include "VulkanComputePass.h"
#include "VulkanComputePipeline.h"

void VulkanInitDescriptorSetLayoutManager(VulkanDescriptorSetLayoutManager* manager, PulseDevice device)
{
	memset(manager, 0, sizeof(VulkanDescriptorSetLayoutManager));
	manager->device = device;
}

VulkanDescriptorSetLayout* VulkanGetDescriptorSetLayout(VulkanDescriptorSetLayoutManager* manager,
														uint32_t read_storage_images_count,
														uint32_t read_storage_buffers_count,
														uint32_t write_storage_images_count,
														uint32_t write_storage_buffers_count,
														uint32_t uniform_buffers_count)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(manager->device, VulkanDevice*);

	for(uint32_t i = 0; i < manager->layouts_size; i++)
	{
		VulkanDescriptorSetLayout* layout = manager->layouts[i];
		if( layout->ReadOnly.storage_buffer_count == read_storage_buffers_count &&
			layout->ReadOnly.storage_texture_count == read_storage_images_count &&
			layout->ReadWrite.storage_buffer_count == write_storage_buffers_count &&
			layout->ReadWrite.storage_texture_count == write_storage_images_count &&
			layout->Uniform.buffer_count == uniform_buffers_count &&
			!layout->is_used)
		{
			layout->is_used = true;
			return layout;
		}
	}

	PULSE_EXPAND_ARRAY_IF_NEEDED(manager->layouts, VulkanDescriptorSetLayout*, manager->layouts_size, manager->layouts_capacity, 1);
	PULSE_CHECK_ALLOCATION_RETVAL(manager->layouts, PULSE_NULLPTR);

	VulkanDescriptorSetLayout* layout = (VulkanDescriptorSetLayout*)malloc(sizeof(VulkanDescriptorSetLayout));
	manager->layouts[manager->layouts_size] = layout;
	manager->layouts_size++;

	VkDescriptorSetLayoutBinding bindings[PULSE_MAX_READ_BUFFERS_BOUND + PULSE_MAX_READ_TEXTURES_BOUND + PULSE_MAX_WRITE_BUFFERS_BOUND + PULSE_MAX_WRITE_TEXTURES_BOUND + PULSE_MAX_UNIFORM_BUFFERS_BOUND] = { 0 };

	uint8_t category;
	if(uniform_buffers_count != 0)
		category = 3;
	else if(write_storage_images_count != 0 || write_storage_buffers_count != 0)
		category = 2;
	else
		category = 1;

	uint32_t count = 0;

	if(category == 1)
	{
		for(uint32_t i = 0; i < read_storage_images_count; i++, count++)
		{
			bindings[i].binding = i;
			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; // Wtf shaders ?
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[i].pImmutableSamplers = PULSE_NULLPTR;
		}

		for(uint32_t i = read_storage_images_count; i < read_storage_images_count + read_storage_buffers_count; i++, count++)
		{
			bindings[i].binding = i;
			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[i].pImmutableSamplers = PULSE_NULLPTR;
		}
	}
	else if(category == 2)
	{
		for(uint32_t i = 0; i < write_storage_images_count; i++, count++)
		{
			bindings[i].binding = i;
			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[i].pImmutableSamplers = PULSE_NULLPTR;
		}

		for(uint32_t i = write_storage_images_count; i < write_storage_images_count + write_storage_buffers_count; i++, count++)
		{
			bindings[i].binding = i;
			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[i].pImmutableSamplers = PULSE_NULLPTR;
		}
	}
	else if(category == 3)
	{
		for(uint32_t i = 0; i < uniform_buffers_count; i++, count++)
		{
			bindings[i].binding = i;
			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[i].pImmutableSamplers = PULSE_NULLPTR;
		}
	}

	VkDescriptorSetLayoutCreateInfo layout_info = { 0 };
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = count;
	layout_info.pBindings = bindings;

	CHECK_VK_RETVAL(manager->device->backend, vulkan_device->vkCreateDescriptorSetLayout(vulkan_device->device, &layout_info, PULSE_NULLPTR, &layout->layout), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULLPTR);

	layout->ReadOnly.storage_buffer_count = read_storage_buffers_count;
	layout->ReadOnly.storage_texture_count = read_storage_images_count;
	layout->ReadWrite.storage_buffer_count = write_storage_buffers_count;
	layout->ReadWrite.storage_texture_count = write_storage_images_count;
	layout->Uniform.buffer_count = uniform_buffers_count;

	layout->is_used = true;

	return layout;
}

void VulkanDestroyDescriptorSetLayout(VulkanDescriptorSetLayout* layout, PulseDevice device)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	vulkan_device->vkDestroyDescriptorSetLayout(vulkan_device->device, layout->layout, PULSE_NULLPTR);
}

void VulkanDestroyDescriptorSetLayoutManager(VulkanDescriptorSetLayoutManager* manager)
{
	for(uint32_t i = 0; i < manager->layouts_size; i++)
		VulkanDestroyDescriptorSetLayout(manager->layouts[i], manager->device);
	free(manager->layouts);
	memset(manager, 0, sizeof(VulkanDescriptorSetPoolManager));
}

void VulkanInitDescriptorSetPool(VulkanDescriptorSetPool* pool, PulseDevice device)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	memset(pool, 0, sizeof(VulkanDescriptorSetPool));
	pool->device = device;
	pool->thread_id = PulseGetThreadID();

	VkDescriptorPoolSize pool_sizes[PULSE_MAX_READ_BUFFERS_BOUND + PULSE_MAX_READ_TEXTURES_BOUND + PULSE_MAX_WRITE_BUFFERS_BOUND + PULSE_MAX_WRITE_TEXTURES_BOUND + PULSE_MAX_UNIFORM_BUFFERS_BOUND] = { 0 };

	uint32_t i = 0;

	for(uint32_t start = i; i < start + PULSE_MAX_READ_TEXTURES_BOUND; i++)
	{
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	for(uint32_t start = i; i < start + PULSE_MAX_READ_BUFFERS_BOUND; i++)
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
		pool_sizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[i].descriptorCount = VULKAN_POOL_SIZE;
	}

	VkDescriptorPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = i;
	pool_info.pPoolSizes = pool_sizes;
	pool_info.maxSets = VULKAN_POOL_SIZE;
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

void VulkanBindDescriptorSets(PulseComputePass pass)
{
	VulkanComputePass* vulkan_pass = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass, VulkanComputePass*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->cmd->device, VulkanDevice*);
	VulkanComputePipeline* vulkan_pipeline = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->current_pipeline, VulkanComputePipeline*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->cmd, VulkanCommandList*);

	if(!vulkan_pass->should_recreate_read_only_descriptor_sets && !vulkan_pass->should_recreate_write_descriptor_sets && !vulkan_pass->should_recreate_uniform_descriptor_sets)
		return;

	VkWriteDescriptorSet writes[PULSE_MAX_READ_BUFFERS_BOUND + PULSE_MAX_READ_TEXTURES_BOUND + PULSE_MAX_WRITE_BUFFERS_BOUND + PULSE_MAX_WRITE_TEXTURES_BOUND + PULSE_MAX_UNIFORM_BUFFERS_BOUND] = { 0 };
	VkDescriptorBufferInfo buffer_infos[PULSE_MAX_UNIFORM_BUFFERS_BOUND + PULSE_MAX_WRITE_BUFFERS_BOUND + PULSE_MAX_READ_BUFFERS_BOUND];
	VkDescriptorImageInfo image_infos[PULSE_MAX_READ_TEXTURES_BOUND + PULSE_MAX_WRITE_TEXTURES_BOUND];
	uint32_t write_count = 0;
	uint32_t buffer_info_count = 0;
	uint32_t image_info_count = 0;

	if(vulkan_pass->should_recreate_read_only_descriptor_sets)
	{
		if(vulkan_pass->read_only_descriptor_set != PULSE_NULLPTR)
			VulkanReturnDescriptorSetToPool(vulkan_pass->read_only_descriptor_set->pool, vulkan_pass->read_only_descriptor_set);
		vulkan_pass->read_only_descriptor_set = VulkanRequestDescriptorSetFromPool(VulkanGetAvailableDescriptorSetPool(&vulkan_device->descriptor_set_pool_manager), vulkan_pipeline->read_only_descriptor_set_layout);

		for(uint32_t i = 0; i < pass->current_pipeline->num_readonly_storage_images; i++)
		{
			if(pass->readonly_images[i] == PULSE_NULL_HANDLE)
				continue;
			VkWriteDescriptorSet* write_descriptor_set = &writes[write_count];

			write_descriptor_set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set->pNext = PULSE_NULLPTR;
			write_descriptor_set->descriptorCount = 1;
			write_descriptor_set->descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; // Wtf shaders ?
			write_descriptor_set->dstArrayElement = 0;
			write_descriptor_set->dstBinding = i;
			write_descriptor_set->dstSet = vulkan_pass->read_only_descriptor_set->set;
			write_descriptor_set->pTexelBufferView = PULSE_NULLPTR;
			write_descriptor_set->pBufferInfo = PULSE_NULLPTR;

			VulkanImage* vulkan_image = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->readonly_images[i], VulkanImage*);

			image_infos[image_info_count].sampler = VK_NULL_HANDLE;
			image_infos[image_info_count].imageView = vulkan_image->view;
			image_infos[image_info_count].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			write_descriptor_set->pImageInfo = &image_infos[image_info_count];

			write_count++;
			image_info_count++;
		}

		for(uint32_t i = 0; i < pass->current_pipeline->num_readonly_storage_buffers; i++)
		{
			if(pass->readonly_storage_buffers[i] == PULSE_NULL_HANDLE)
				continue;
			VkWriteDescriptorSet* write_descriptor_set = &writes[write_count];

			write_descriptor_set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set->pNext = PULSE_NULLPTR;
			write_descriptor_set->descriptorCount = 1;
			write_descriptor_set->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write_descriptor_set->dstArrayElement = 0;
			write_descriptor_set->dstBinding = pass->current_pipeline->num_readonly_storage_images + i;
			write_descriptor_set->dstSet = vulkan_pass->read_only_descriptor_set->set;
			write_descriptor_set->pTexelBufferView = PULSE_NULLPTR;
			write_descriptor_set->pBufferInfo = PULSE_NULLPTR;

			VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->readonly_storage_buffers[i], VulkanBuffer*);

			buffer_infos[buffer_info_count].buffer = vulkan_buffer->buffer;
			buffer_infos[buffer_info_count].offset = 0;
			buffer_infos[buffer_info_count].range = VK_WHOLE_SIZE;

			write_descriptor_set->pBufferInfo = &buffer_infos[buffer_info_count];

			write_count++;
			buffer_info_count++;
		}

		vulkan_pass->should_recreate_read_only_descriptor_sets = false;
	}

	if(vulkan_pass->should_recreate_write_descriptor_sets)
	{
		if(vulkan_pass->read_write_descriptor_set != PULSE_NULLPTR)
			VulkanReturnDescriptorSetToPool(vulkan_pass->read_write_descriptor_set->pool, vulkan_pass->read_write_descriptor_set);
		vulkan_pass->read_write_descriptor_set = VulkanRequestDescriptorSetFromPool(VulkanGetAvailableDescriptorSetPool(&vulkan_device->descriptor_set_pool_manager), vulkan_pipeline->read_write_descriptor_set_layout);

		for(uint32_t i = 0; i < pass->current_pipeline->num_readwrite_storage_images; i++)
		{
			if(pass->readwrite_images[i] == PULSE_NULL_HANDLE)
				continue;
			VkWriteDescriptorSet* write_descriptor_set = &writes[write_count];

			write_descriptor_set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set->pNext = PULSE_NULLPTR;
			write_descriptor_set->descriptorCount = 1;
			write_descriptor_set->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write_descriptor_set->dstArrayElement = 0;
			write_descriptor_set->dstBinding = i;
			write_descriptor_set->dstSet = vulkan_pass->read_write_descriptor_set->set;
			write_descriptor_set->pTexelBufferView = PULSE_NULLPTR;
			write_descriptor_set->pBufferInfo = PULSE_NULLPTR;

			VulkanImage* vulkan_image = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->readwrite_images[i], VulkanImage*);

			image_infos[image_info_count].sampler = VK_NULL_HANDLE;
			image_infos[image_info_count].imageView = vulkan_image->view;
			image_infos[image_info_count].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			write_descriptor_set->pImageInfo = &image_infos[image_info_count];

			write_count++;
			image_info_count++;
		}

		for(uint32_t i = 0; i < pass->current_pipeline->num_readwrite_storage_buffers; i++)
		{
			if(pass->readwrite_storage_buffers[i] == PULSE_NULL_HANDLE)
				continue;
			VkWriteDescriptorSet* write_descriptor_set = &writes[write_count];

			write_descriptor_set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set->pNext = PULSE_NULLPTR;
			write_descriptor_set->descriptorCount = 1;
			write_descriptor_set->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write_descriptor_set->dstArrayElement = 0;
			write_descriptor_set->dstBinding = pass->current_pipeline->num_readwrite_storage_images + i;
			write_descriptor_set->dstSet = vulkan_pass->read_write_descriptor_set->set;
			write_descriptor_set->pTexelBufferView = PULSE_NULLPTR;
			write_descriptor_set->pBufferInfo = PULSE_NULLPTR;

			VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->readwrite_storage_buffers[i], VulkanBuffer*);

			buffer_infos[buffer_info_count].buffer = vulkan_buffer->buffer;
			buffer_infos[buffer_info_count].offset = 0;
			buffer_infos[buffer_info_count].range = VK_WHOLE_SIZE;

			write_descriptor_set->pBufferInfo = &buffer_infos[buffer_info_count];

			write_count++;
			buffer_info_count++;
		}

		vulkan_pass->should_recreate_write_descriptor_sets = false;
	}

	if(vulkan_pass->should_recreate_uniform_descriptor_sets)
	{
		if(vulkan_pass->uniform_descriptor_set != PULSE_NULLPTR)
			VulkanReturnDescriptorSetToPool(vulkan_pass->uniform_descriptor_set->pool, vulkan_pass->uniform_descriptor_set);
		vulkan_pass->uniform_descriptor_set = VulkanRequestDescriptorSetFromPool(VulkanGetAvailableDescriptorSetPool(&vulkan_device->descriptor_set_pool_manager), vulkan_pipeline->uniform_descriptor_set_layout);

		for(uint32_t i = 0; i < pass->current_pipeline->num_uniform_buffers; i++)
		{
			VkWriteDescriptorSet* write_descriptor_set = &writes[write_count];

			write_descriptor_set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set->pNext = PULSE_NULLPTR;
			write_descriptor_set->descriptorCount = 1;
			write_descriptor_set->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_descriptor_set->dstArrayElement = 0;
			write_descriptor_set->dstBinding = i;
			write_descriptor_set->dstSet = vulkan_pass->uniform_descriptor_set->set;
			write_descriptor_set->pTexelBufferView = PULSE_NULLPTR;
			write_descriptor_set->pBufferInfo = PULSE_NULLPTR;

			VulkanBuffer* vulkan_buffer = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->uniform_buffers[i], VulkanBuffer*);

			buffer_infos[buffer_info_count].buffer = vulkan_buffer->buffer;
			buffer_infos[buffer_info_count].offset = 0;
			buffer_infos[buffer_info_count].range = VK_WHOLE_SIZE;

			write_descriptor_set->pBufferInfo = &buffer_infos[buffer_info_count];

			write_count++;
			buffer_info_count++;
		}

		vulkan_pass->should_recreate_uniform_descriptor_sets = false;
	}

	vulkan_device->vkUpdateDescriptorSets(vulkan_device->device, write_count, writes, 0, PULSE_NULLPTR);

	VkDescriptorSet sets[3];
	sets[0] = vulkan_pass->read_only_descriptor_set->set;
	sets[1] = vulkan_pass->read_write_descriptor_set->set;
	sets[2] = vulkan_pass->uniform_descriptor_set->set;

	vulkan_device->vkCmdBindDescriptorSets(vulkan_cmd->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vulkan_pipeline->layout, 0, 3, sets, 0, PULSE_NULLPTR);
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
	PulseThreadID thread_id = PulseGetThreadID();
	for(uint32_t i = 0; i < manager->pools_size; i++)
	{
		if(thread_id == manager->pools[i]->thread_id && (manager->pools[i]->allocations_count < VULKAN_POOL_SIZE || manager->pools[i]->free_sets[0] != PULSE_NULLPTR))
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
