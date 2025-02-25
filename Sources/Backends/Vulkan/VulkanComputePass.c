// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanComputePass.h"
#include "VulkanComputePipeline.h"

PulseComputePass VulkanCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PULSE_UNUSED(device);
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	VulkanComputePass* vulkan_pass = (VulkanComputePass*)calloc(1, sizeof(VulkanComputePass));
	PULSE_CHECK_ALLOCATION_RETVAL(vulkan_pass, PULSE_NULL_HANDLE);

	pass->cmd = cmd;
	pass->driver_data = vulkan_pass;

	return pass;
}

void VulkanDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
	PULSE_UNUSED(device);
	free(pass->driver_data);
	free(pass);
}

void VulkanBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
	PulseBufferUsageFlags usage = buffers[0]->usage;
	bool is_readwrite = (usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) != 0;
	PulseBuffer* array = is_readwrite ? pass->readwrite_storage_buffers : pass->readonly_storage_buffers;
	VulkanComputePass* vulkan_pass = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass, VulkanComputePass*);

	for(uint32_t i = 0; i < num_buffers; i++)
	{
		if(is_readwrite && (buffers[i]->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) == 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read only buffer with read-write buffers");
			PulseSetInternalError(PULSE_ERROR_INVALID_BUFFER_USAGE);
			return;
		}
		else if(!is_readwrite && (buffers[i]->usage & PULSE_BUFFER_USAGE_STORAGE_WRITE) != 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read-write buffer with read only buffers");
			PulseSetInternalError(PULSE_ERROR_INVALID_BUFFER_USAGE);
			return;
		}

		if(array[i] == buffers[i])
			continue;
		array[i] = buffers[i];

		if(is_readwrite)
			vulkan_pass->should_recreate_write_descriptor_sets = true;
		else
			vulkan_pass->should_recreate_read_only_descriptor_sets = true;
	}
}

void VulkanBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void VulkanBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
	PulseImageUsageFlags usage = images[0]->usage;
	bool is_readwrite = (usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0;
	PulseImage* array = is_readwrite ? pass->readwrite_images : pass->readonly_images;
	VulkanComputePass* vulkan_pass = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass, VulkanComputePass*);

	for(uint32_t i = 0; i < num_images; i++)
	{
		if(is_readwrite && (images[i]->usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) == 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read only image with read-write images");
			PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_USAGE);
			return;
		}
		else if(!is_readwrite && (images[i]->usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(pass->cmd->device->backend))
				PulseLogError(pass->cmd->device->backend, "cannot bind a read-write image with read only images");
			PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_USAGE);
			return;
		}

		if(array[i] == images[i])
			continue;
		array[i] = images[i];
		
		if((usage & PULSE_IMAGE_USAGE_STORAGE_WRITE) != 0)
			vulkan_pass->should_recreate_write_descriptor_sets = true;
		else
			vulkan_pass->should_recreate_read_only_descriptor_sets = true;
	}
}

void VulkanBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
	VulkanComputePipeline* vulkan_pipeline = VULKAN_RETRIEVE_DRIVER_DATA_AS(pipeline, VulkanComputePipeline*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->cmd->device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->cmd, VulkanCommandList*);
	VulkanComputePass* vulkan_pass = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass, VulkanComputePass*);

	vulkan_device->vkCmdBindPipeline(vulkan_cmd->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vulkan_pipeline->pipeline);

	vulkan_pass->should_recreate_read_only_descriptor_sets = true;
	vulkan_pass->should_recreate_write_descriptor_sets = true;
	vulkan_pass->should_recreate_uniform_descriptor_sets = true;
}

void VulkanDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->cmd->device, VulkanDevice*);
	VulkanCommandList* vulkan_cmd = VULKAN_RETRIEVE_DRIVER_DATA_AS(pass->cmd, VulkanCommandList*);

	VulkanBindDescriptorSets(pass);

	vulkan_device->vkCmdDispatch(vulkan_cmd->cmd, groupcount_x, groupcount_y, groupcount_z);
}

PulseComputePass VulkanBeginComputePass(PulseCommandList cmd)
{
	return cmd->pass;
}

void VulkanEndComputePass(PulseComputePass pass)
{
}
