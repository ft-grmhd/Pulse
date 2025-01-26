// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanCommandList.h"
#include "VulkanComputePipeline.h"

PulseComputePipeline VulkanCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);

	PulseComputePipelineHandler* pipeline = (PulseComputePipelineHandler*)calloc(1, sizeof(PulseComputePipelineHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pipeline, PULSE_NULL_HANDLE);

	VulkanComputePipeline* vulkan_pipeline = (VulkanComputePipeline*)calloc(1, sizeof(VulkanComputePipeline));
	PULSE_CHECK_ALLOCATION_RETVAL(vulkan_pipeline, PULSE_NULL_HANDLE);

	pipeline->driver_data = vulkan_pipeline;

	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(info->code == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid code pointer passed to PulseComputePipelineCreateInfo");
		if(info->entrypoint == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid entrypoint pointer passed to PulseComputePipelineCreateInfo");
		if(info->format == PULSE_SHADER_FORMAT_SPIRV_BIT && (device->backend->supported_shader_formats & PULSE_SHADER_FORMAT_SPIRV_BIT) == 0)
			PulseLogError(device->backend, "invalid shader format passed to PulseComputePipelineCreateInfo");
	}

	VkShaderModuleCreateInfo shader_module_create_info = {};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = info->code_size;
	shader_module_create_info.pCode = (const uint32_t*)info->code;
	CHECK_VK_RETVAL(device->backend, vulkan_device->vkCreateShaderModule(vulkan_device->device, &shader_module_create_info, PULSE_NULLPTR, &vulkan_pipeline->module), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);

	VkPipelineShaderStageCreateInfo shader_stage_info = {};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage_info.module = vulkan_pipeline->module;
	shader_stage_info.pName = info->entrypoint;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = PULSE_NULLPTR; // will change
	CHECK_VK_RETVAL(device->backend, vulkan_device->vkCreatePipelineLayout(vulkan_device->device, &pipeline_layout_info, PULSE_NULLPTR, &vulkan_pipeline->layout), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);

	VkComputePipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.layout = vulkan_pipeline->layout;
	pipeline_info.stage = shader_stage_info;

	CHECK_VK_RETVAL(device->backend, vulkan_device->vkCreateComputePipelines(vulkan_device->device, VK_NULL_HANDLE, 1, &pipeline_info, PULSE_NULLPTR, &vulkan_pipeline->pipeline), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Vulkan) created new compute pipeline %p", pipeline);

	return pipeline;
}

void VulkanDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	if(pipeline == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "compute pipeline is NULL, this may be a bug in your application");
		return;
	}

	VulkanComputePipeline* vulkan_pipeline = VULKAN_RETRIEVE_DRIVER_DATA_AS(pipeline, VulkanComputePipeline*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	vulkan_device->vkDeviceWaitIdle(vulkan_device->device);
	vulkan_device->vkDestroyShaderModule(vulkan_device->device, vulkan_pipeline->module, PULSE_NULLPTR);
	vulkan_device->vkDestroyPipelineLayout(vulkan_device->device, vulkan_pipeline->layout, PULSE_NULLPTR);
	vulkan_device->vkDestroyPipeline(vulkan_device->device, vulkan_pipeline->pipeline, PULSE_NULLPTR);
	free(vulkan_pipeline);
	free(pipeline);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Vulkan) destroyed compute pipeline %p", pipeline);
}
