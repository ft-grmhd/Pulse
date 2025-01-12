// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "Vulkan.h"
#include "VulkanImage.h"
#include "VulkanDevice.h"
#include <vulkan/vulkan_core.h>

static VkFormat PulseImageFormatToVkFormat[] = {
	VK_FORMAT_UNDEFINED,                   // INVALID
	VK_FORMAT_R8_UNORM,                    // A8_UNORM
	VK_FORMAT_R8_UNORM,                    // R8_UNORM
	VK_FORMAT_R8G8_UNORM,                  // R8G8_UNORM
	VK_FORMAT_R8G8B8A8_UNORM,              // R8G8B8A8_UNORM
	VK_FORMAT_R16_UNORM,                   // R16_UNORM
	VK_FORMAT_R16G16_UNORM,                // R16G16_UNORM
	VK_FORMAT_R16G16B16A16_UNORM,          // R16G16B16A16_UNORM
	VK_FORMAT_A2B10G10R10_UNORM_PACK32,    // R10G10B10A2_UNORM
	VK_FORMAT_R5G6B5_UNORM_PACK16,         // B5G6R5_UNORM
	VK_FORMAT_A1R5G5B5_UNORM_PACK16,       // B5G5R5A1_UNORM
	VK_FORMAT_B4G4R4A4_UNORM_PACK16,       // B4G4R4A4_UNORM
	VK_FORMAT_B8G8R8A8_UNORM,              // B8G8R8A8_UNORM
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,        // BC1_UNORM
	VK_FORMAT_BC2_UNORM_BLOCK,             // BC2_UNORM
	VK_FORMAT_BC3_UNORM_BLOCK,             // BC3_UNORM
	VK_FORMAT_BC4_UNORM_BLOCK,             // BC4_UNORM
	VK_FORMAT_BC5_UNORM_BLOCK,             // BC5_UNORM
	VK_FORMAT_BC7_UNORM_BLOCK,             // BC7_UNORM
	VK_FORMAT_BC6H_SFLOAT_BLOCK,           // BC6H_FLOAT
	VK_FORMAT_BC6H_UFLOAT_BLOCK,           // BC6H_UFLOAT
	VK_FORMAT_R8_SNORM,                    // R8_SNORM
	VK_FORMAT_R8G8_SNORM,                  // R8G8_SNORM
	VK_FORMAT_R8G8B8A8_SNORM,              // R8G8B8A8_SNORM
	VK_FORMAT_R16_SNORM,                   // R16_SNORM
	VK_FORMAT_R16G16_SNORM,                // R16G16_SNORM
	VK_FORMAT_R16G16B16A16_SNORM,          // R16G16B16A16_SNORM
	VK_FORMAT_R16_SFLOAT,                  // R16_FLOAT
	VK_FORMAT_R16G16_SFLOAT,               // R16G16_FLOAT
	VK_FORMAT_R16G16B16A16_SFLOAT,         // R16G16B16A16_FLOAT
	VK_FORMAT_R32_SFLOAT,                  // R32_FLOAT
	VK_FORMAT_R32G32_SFLOAT,               // R32G32_FLOAT
	VK_FORMAT_R32G32B32A32_SFLOAT,         // R32G32B32A32_FLOAT
	VK_FORMAT_B10G11R11_UFLOAT_PACK32,     // R11G11B10_UFLOAT
	VK_FORMAT_R8_UINT,                     // R8_UINT
	VK_FORMAT_R8G8_UINT,                   // R8G8_UINT
	VK_FORMAT_R8G8B8A8_UINT,               // R8G8B8A8_UINT
	VK_FORMAT_R16_UINT,                    // R16_UINT
	VK_FORMAT_R16G16_UINT,                 // R16G16_UINT
	VK_FORMAT_R16G16B16A16_UINT,           // R16G16B16A16_UINT
	VK_FORMAT_R32_UINT,                    // R32_UINT
	VK_FORMAT_R32G32_UINT,                 // R32G32_UINT
	VK_FORMAT_R32G32B32A32_UINT,           // R32G32B32A32_UINT
	VK_FORMAT_R8_SINT,                     // R8_INT
	VK_FORMAT_R8G8_SINT,                   // R8G8_INT
	VK_FORMAT_R8G8B8A8_SINT,               // R8G8B8A8_INT
	VK_FORMAT_R16_SINT,                    // R16_INT
	VK_FORMAT_R16G16_SINT,                 // R16G16_INT
	VK_FORMAT_R16G16B16A16_SINT,           // R16G16B16A16_INT
	VK_FORMAT_R32_SINT,                    // R32_INT
	VK_FORMAT_R32G32_SINT,                 // R32G32_INT
	VK_FORMAT_R32G32B32A32_SINT,           // R32G32B32A32_INT
	VK_FORMAT_R8G8B8A8_SRGB,               // R8G8B8A8_UNORM_SRGB
	VK_FORMAT_B8G8R8A8_SRGB,               // B8G8R8A8_UNORM_SRGB
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,         // BC1_UNORM_SRGB
	VK_FORMAT_BC2_SRGB_BLOCK,              // BC3_UNORM_SRGB
	VK_FORMAT_BC3_SRGB_BLOCK,              // BC3_UNORM_SRGB
	VK_FORMAT_BC7_SRGB_BLOCK,              // BC7_UNORM_SRGB
};
PULSE_STATIC_ASSERT(PulseImageFormatToVkFormat, (sizeof(PulseImageFormatToVkFormat) / sizeof(VkFormat)) == PULSE_IMAGE_FORMAT_MAX_ENUM);

static VkComponentMapping SwizzleForFormat(PulseImageFormat format)
{
	if(format == PULSE_IMAGE_FORMAT_A8_UNORM)
	{
		return (VkComponentMapping){
			VK_COMPONENT_SWIZZLE_ZERO,
			VK_COMPONENT_SWIZZLE_ZERO,
			VK_COMPONENT_SWIZZLE_ZERO,
			VK_COMPONENT_SWIZZLE_R
		};
	}

	if(format == PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM)
	{
		return (VkComponentMapping){
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_A,
			VK_COMPONENT_SWIZZLE_B
		};
	}

	return (VkComponentMapping){
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
}

PulseImage VulkanCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);

	PulseImageHandler* image = (PulseImageHandler*)calloc(1, sizeof(PulseImageHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(image, PULSE_NULL_HANDLE);

	VulkanImage* vulkan_image = (VulkanImage*)calloc(1, sizeof(VulkanImage));
	PULSE_CHECK_ALLOCATION_RETVAL(vulkan_image, PULSE_NULL_HANDLE);

	uint32_t layer_count = (create_infos->type == PULSE_IMAGE_TYPE_3D) ? 1 : create_infos->layer_count_or_depth;
	uint32_t depth = (create_infos->type == PULSE_IMAGE_TYPE_3D) ? create_infos->layer_count_or_depth : 1;

	image->device = device;
	image->driver_data = vulkan_image;

	VmaAllocationCreateInfo allocation_create_info = { 0 };
	allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

	VkImageCreateFlags flags = 0;

	if(create_infos->type == PULSE_IMAGE_TYPE_CUBE || create_infos->type == PULSE_IMAGE_TYPE_CUBE_ARRAY)
		flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	else if(create_infos->type == PULSE_IMAGE_TYPE_3D)
		flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

	VkImageCreateInfo image_create_info = { 0 };
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.extent.width = create_infos->width;
	image_create_info.extent.height = create_infos->height;
	image_create_info.extent.depth = depth;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = layer_count;
	image_create_info.format = PulseImageFormatToVkFormat[create_infos->format];
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = flags;

	CHECK_VK_RETVAL(device->backend, vmaCreateImage(vulkan_device->allocator, &image_create_info, &allocation_create_info, &vulkan_image->image, &vulkan_image->allocation, PULSE_NULLPTR), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);
	vmaGetAllocationInfo(vulkan_device->allocator, vulkan_image->allocation, &vulkan_image->allocation_info);

	if(create_infos->usage & PULSE_IMAGE_USAGE_STORAGE_READ)
	{
		VkImageViewCreateInfo image_view_create_info = { 0 };
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = vulkan_image->image;
		image_view_create_info.format = PulseImageFormatToVkFormat[create_infos->format];
		image_view_create_info.components = SwizzleForFormat(create_infos->format);
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = layer_count;

		if(create_infos->type == PULSE_IMAGE_TYPE_CUBE)
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		else if(create_infos->type == PULSE_IMAGE_TYPE_CUBE_ARRAY)
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		else if(create_infos->type == PULSE_IMAGE_TYPE_3D)
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
		else if(create_infos->type == PULSE_IMAGE_TYPE_2D_ARRAY)
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		else
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

		CHECK_VK_RETVAL(device->backend, vulkan_device->vkCreateImageView(vulkan_device->device, &image_view_create_info, PULSE_NULLPTR, &vulkan_image->view), PULSE_ERROR_INITIALIZATION_FAILED, PULSE_NULL_HANDLE);
	}

	return image;
}

bool VulkanIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage)
{
	(void)usage;
	VulkanDriverData* vulkan_driver_data = VULKAN_RETRIEVE_DRIVER_DATA_AS(device->backend, VulkanDriverData*);
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	VkImageCreateFlags vulkan_flags = 0;
	VkImageFormatProperties properties;
	if(type == PULSE_IMAGE_TYPE_CUBE || type == PULSE_IMAGE_TYPE_CUBE_ARRAY)
		vulkan_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	return vulkan_driver_data->instance.vkGetPhysicalDeviceImageFormatProperties(vulkan_device->physical, PulseImageFormatToVkFormat[format], (type == PULSE_IMAGE_TYPE_3D) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, vulkan_flags, &properties) == VK_SUCCESS;
}

bool VulkanCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst)
{
}

bool VulkanBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst)
{
}

void VulkanDestroyImage(PulseDevice device, PulseImage image)
{
	VulkanDevice* vulkan_device = VULKAN_RETRIEVE_DRIVER_DATA_AS(device, VulkanDevice*);
	VulkanImage* vulkan_image = VULKAN_RETRIEVE_DRIVER_DATA_AS(image, VulkanImage*);
	vulkan_device->vkDestroyImageView(vulkan_device->device, vulkan_image->view, PULSE_NULLPTR);
	vmaDestroyImage(vulkan_device->allocator, vulkan_image->image, vulkan_image->allocation);
	free(vulkan_image);
	free(image);
}
