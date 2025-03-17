// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "WebGPUImage.h"
#include "WebGPUCommandList.h"

static WGPUTextureFormat PulseImageFormatToWGPUTextureFormat[] = {
	WGPUTextureFormat_Undefined,         // INVALID
	WGPUTextureFormat_R8Unorm,           // A8_UNORM
	WGPUTextureFormat_R8Unorm,           // R8_UNORM
	WGPUTextureFormat_RG8Unorm,          // R8G8_UNORM
	WGPUTextureFormat_RGBA8Unorm,        // R8G8B8A8_UNORM
	WGPUTextureFormat_R16Uint,           // R16_UNORM
	WGPUTextureFormat_RG16Uint,          // R16G16_UNORM
	WGPUTextureFormat_RGBA16Uint,        // R16G16B16A16_UNORM
	WGPUTextureFormat_RGB10A2Unorm,      // R10G10B10A2_UNORM
	WGPUTextureFormat_Undefined,         // B5G6R5_UNORM
	WGPUTextureFormat_Undefined,         // B5G5R5A1_UNORM
	WGPUTextureFormat_Undefined,         // B4G4R4A4_UNORM
	WGPUTextureFormat_BGRA8Unorm,        // B8G8R8A8_UNORM
	WGPUTextureFormat_BC1RGBAUnorm,      // BC1_UNORM
	WGPUTextureFormat_BC2RGBAUnorm,      // BC2_UNORM
	WGPUTextureFormat_BC3RGBAUnorm,      // BC3_UNORM
	WGPUTextureFormat_BC4RUnorm,         // BC4_UNORM
	WGPUTextureFormat_BC5RGUnorm,        // BC5_UNORM
	WGPUTextureFormat_BC7RGBAUnorm,      // BC7_UNORM
	WGPUTextureFormat_BC6HRGBFloat,      // BC6H_FLOAT
	WGPUTextureFormat_BC6HRGBUfloat,     // BC6H_UFLOAT
	WGPUTextureFormat_R8Snorm,           // R8_SNORM
	WGPUTextureFormat_RG8Snorm,          // R8G8_SNORM
	WGPUTextureFormat_RGBA8Snorm,        // R8G8B8A8_SNORM
	WGPUTextureFormat_R16Sint,           // R16_SNORM
	WGPUTextureFormat_RG16Sint,          // R16G16_SNORM
	WGPUTextureFormat_RGBA16Sint,        // R16G16B16A16_SNORM
	WGPUTextureFormat_R16Float,          // R16_FLOAT
	WGPUTextureFormat_RG16Float,         // R16G16_FLOAT
	WGPUTextureFormat_RGBA16Float,       // R16G16B16A16_FLOAT
	WGPUTextureFormat_R32Float,          // R32_FLOAT
	WGPUTextureFormat_RG32Float,         // R32G32_FLOAT
	WGPUTextureFormat_RGBA32Float,       // R32G32B32A32_FLOAT
	WGPUTextureFormat_RG11B10Ufloat,     // R11G11B10_UFLOAT
	WGPUTextureFormat_R8Uint,            // R8_UINT
	WGPUTextureFormat_RG8Unorm,          // R8G8_UINT
	WGPUTextureFormat_RGBA8Uint,         // R8G8B8A8_UINT
	WGPUTextureFormat_R16Uint,           // R16_UINT
	WGPUTextureFormat_RG16Uint,          // R16G16_UINT
	WGPUTextureFormat_RGBA16Uint,        // R16G16B16A16_UINT
	WGPUTextureFormat_R32Uint,           // R32_UINT
	WGPUTextureFormat_RG32Uint,          // R32G32_UINT
	WGPUTextureFormat_RGBA32Uint,        // R32G32B32A32_UINT
	WGPUTextureFormat_R8Sint,            // R8_INT
	WGPUTextureFormat_RG8Sint,           // R8G8_INT
	WGPUTextureFormat_RGBA8Sint,         // R8G8B8A8_INT
	WGPUTextureFormat_R16Sint,           // R16_INT
	WGPUTextureFormat_RG16Sint,          // R16G16_INT
	WGPUTextureFormat_RGBA16Sint,        // R16G16B16A16_INT
	WGPUTextureFormat_R32Sint,           // R32_INT
	WGPUTextureFormat_RG32Sint,          // R32G32_INT
	WGPUTextureFormat_RGBA32Sint,        // R32G32B32A32_INT
};
PULSE_STATIC_ASSERT(PulseImageFormatToWGPUTextureFormat, (sizeof(PulseImageFormatToWGPUTextureFormat) / sizeof(WGPUTextureFormat)) == PULSE_IMAGE_FORMAT_MAX_ENUM);

static WGPUTextureDimension PulseImageTypeToWGPUTextureDimension[] = {
	WGPUTextureDimension_2D,    //PULSE_IMAGE_TYPE_2D
	WGPUTextureDimension_2D,    //PULSE_IMAGE_TYPE_2D_ARRAY
	WGPUTextureDimension_3D,    //PULSE_IMAGE_TYPE_3D
	WGPUTextureDimension_2D,    //PULSE_IMAGE_TYPE_CUBE
	WGPUTextureDimension_2D,    //PULSE_IMAGE_TYPE_CUBE_ARRAY
};
PULSE_STATIC_ASSERT(PulseImageTypeToWGPUTextureDimension, (sizeof(PulseImageTypeToWGPUTextureDimension) / sizeof(WGPUTextureDimension)) == PULSE_IMAGE_TYPE_MAX_ENUM);

static WGPUTextureViewDimension PulseImageTypeToWGPUTextureViewDimension[] = {
	WGPUTextureViewDimension_2D,          //PULSE_IMAGE_TYPE_2D
	WGPUTextureViewDimension_2DArray,     //PULSE_IMAGE_TYPE_2D_ARRAY
	WGPUTextureViewDimension_3D,          //PULSE_IMAGE_TYPE_3D
	WGPUTextureViewDimension_Cube,        //PULSE_IMAGE_TYPE_CUBE
	WGPUTextureViewDimension_CubeArray,   //PULSE_IMAGE_TYPE_CUBE_ARRAY
};
PULSE_STATIC_ASSERT(PulseImageTypeToWGPUTextureViewDimension, (sizeof(PulseImageTypeToWGPUTextureViewDimension) / sizeof(WGPUTextureViewDimension)) == PULSE_IMAGE_TYPE_MAX_ENUM);

PulseImage WebGPUCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);

	PulseImageHandler* image = (PulseImageHandler*)calloc(1, sizeof(PulseImageHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(image, PULSE_NULL_HANDLE);

	WebGPUImage* webgpu_image = (WebGPUImage*)calloc(1, sizeof(WebGPUImage));
	PULSE_CHECK_ALLOCATION_RETVAL(webgpu_image, PULSE_NULL_HANDLE);

	WGPUTextureDescriptor descriptor = { 0 };
	descriptor.dimension = PulseImageTypeToWGPUTextureDimension[create_infos->type];
	descriptor.format = PulseImageFormatToWGPUTextureFormat[create_infos->format];

	if(create_infos->format == PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM || create_infos->format == PULSE_IMAGE_FORMAT_B5G6R5_UNORM || create_infos->format == PULSE_IMAGE_FORMAT_B5G5R5A1_UNORM)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogError(device->backend, "(WebGPU) unsupported image format");
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		return PULSE_NULL_HANDLE;
	}

	descriptor.viewFormatCount = 1;
	descriptor.viewFormats = &descriptor.format;
	descriptor.size.width = create_infos->width;
	descriptor.size.height = create_infos->height;
	descriptor.mipLevelCount = 1;
	descriptor.sampleCount = 1;

	if(create_infos->type == PULSE_IMAGE_TYPE_3D)
		descriptor.size.depthOrArrayLayers = create_infos->layer_count_or_depth;
	else if(create_infos->type == PULSE_IMAGE_TYPE_2D_ARRAY || create_infos->type == PULSE_IMAGE_TYPE_CUBE || create_infos->type == PULSE_IMAGE_TYPE_CUBE_ARRAY)
		descriptor.size.depthOrArrayLayers = create_infos->layer_count_or_depth;
	else
		descriptor.size.depthOrArrayLayers = 1;

	descriptor.usage = 0;
	if(create_infos->usage & PULSE_IMAGE_USAGE_STORAGE_READ)
		descriptor.usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopyDst;
	if(create_infos->usage & PULSE_IMAGE_USAGE_STORAGE_WRITE)
		descriptor.usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc;
	if(create_infos->usage & PULSE_IMAGE_USAGE_STORAGE_SIMULTANEOUS_READWRITE)
		descriptor.usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;

	webgpu_image->texture = wgpuDeviceCreateTexture(webgpu_device->device, &descriptor);
	if(webgpu_image->texture == PULSE_NULLPTR)
	{
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		free(webgpu_image);
		free(image);
		return PULSE_NULL_HANDLE;
	}

	WGPUTextureViewDescriptor view_descriptor = { 0 };
	view_descriptor.format = descriptor.format;
	view_descriptor.dimension = PulseImageTypeToWGPUTextureViewDimension[create_infos->type];
	view_descriptor.baseMipLevel = 0;
	view_descriptor.mipLevelCount = 1;
	view_descriptor.baseArrayLayer = 0;
	view_descriptor.arrayLayerCount = view_descriptor.dimension == WGPUTextureViewDimension_3D ? 1 : create_infos->layer_count_or_depth;
	webgpu_image->view = wgpuTextureCreateView(webgpu_image->texture, &view_descriptor);
	if(webgpu_image->texture == PULSE_NULLPTR)
	{
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		wgpuTextureRelease(webgpu_image->texture);
		free(webgpu_image);
		free(image);
		return PULSE_NULL_HANDLE;
	}

	return image;
}

bool WebGPUIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage)
{
	WGPUTextureFormat wgpu_format = PulseImageFormatToWGPUTextureFormat[format];

	WGPUTextureDimension dimension = WGPUTextureDimension_Undefined;
	if(type == PULSE_IMAGE_TYPE_2D || type == PULSE_IMAGE_TYPE_2D_ARRAY)
		dimension = WGPUTextureDimension_2D;
	else if(type == PULSE_IMAGE_TYPE_3D || type == PULSE_IMAGE_TYPE_CUBE || type == PULSE_IMAGE_TYPE_CUBE_ARRAY)
		dimension = WGPUTextureDimension_3D;

	WGPUTextureUsage wgpu_usage = 0;
	if(usage & PULSE_IMAGE_USAGE_STORAGE_READ)
		wgpu_usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopyDst;
	if(usage & PULSE_IMAGE_USAGE_STORAGE_WRITE)
		wgpu_usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc;
	if(usage & PULSE_IMAGE_USAGE_STORAGE_SIMULTANEOUS_READWRITE)
		wgpu_usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;

	if(wgpu_format == WGPUTextureFormat_Undefined)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogError(device->backend, "(WebGPU) unsupported image format");
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		return false;
	}
	if(wgpu_usage == WGPUTextureUsage_None)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogError(device->backend, "(WebGPU) unsupported image usage");
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_USAGE);
		return false;
	}
	if(dimension == WGPUTextureDimension_Undefined)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogError(device->backend, "(WebGPU) unsupported image type");
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		return false;
	}
	return true;
}

bool WebGPUCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst)
{
	WebGPUImage* webgpu_src_image = WEBGPU_RETRIEVE_DRIVER_DATA_AS(src->image, WebGPUImage*);
	WebGPUBuffer* webgpu_dst_buffer = WEBGPU_RETRIEVE_DRIVER_DATA_AS(dst->buffer, WebGPUBuffer*);
	WebGPUCommandList* webgpu_cmd = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd, WebGPUCommandList*);

	PulseImageFormat format = src->image->format;
	uint32_t block_height = WebGPUGetImageBlockHeight(format) > 1 ? WebGPUGetImageBlockHeight(format) : 1;
	uint32_t blocks_per_column = (src->image->height + block_height - 1) / block_height;
	uint32_t bytes_per_row = WebGPUBytesPerRow(src->image->width, src->image->format);

	if(bytes_per_row == 0)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(cmd->device->backend))
			PulseLogError(cmd->device->backend, "(WebGPU) unsupported image format");
		PulseSetInternalError(PULSE_ERROR_INVALID_IMAGE_FORMAT);
		return false;
	}

	WGPUTexelCopyBufferLayout layout = { 0 };
	layout.bytesPerRow = bytes_per_row;
	layout.rowsPerImage = blocks_per_column;

	WGPUTexelCopyTextureInfo texture_copy_info = { 0 };
	texture_copy_info.texture = webgpu_src_image->texture;
	texture_copy_info.mipLevel = 1;
	texture_copy_info.aspect = WGPUTextureAspect_All;
	texture_copy_info.origin.x = src->x;
	texture_copy_info.origin.y = src->y;
	texture_copy_info.origin.z = src->z;

	WGPUTexelCopyBufferInfo buffer_copy_info = { 0 };
	buffer_copy_info.buffer = webgpu_dst_buffer->buffer;
	buffer_copy_info.layout = layout;

	WGPUExtent3D extent = { 0 };
	extent.width = src->width;
	extent.height = src->height;
	extent.depthOrArrayLayers = src->depth;

	wgpuCommandEncoderCopyTextureToBuffer(webgpu_cmd->encoder, &texture_copy_info, &buffer_copy_info, &extent);

	return true;
}

bool WebGPUBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst)
{
}

void WebGPUDestroyImage(PulseDevice device, PulseImage image)
{
	PULSE_UNUSED(device);
	WebGPUImage* webgpu_image = WEBGPU_RETRIEVE_DRIVER_DATA_AS(image, WebGPUImage*);
	wgpuTextureViewRelease(webgpu_image->view);
	wgpuTextureRelease(webgpu_image->texture);
	free(webgpu_image);
	free(image);
}
