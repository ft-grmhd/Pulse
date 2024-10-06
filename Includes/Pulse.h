// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef PULSE_H_
#define PULSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "PulseProfile.h"

#define PULSE_VERSION PULSE_MAKE_VERSION(1, 0, 0)

// Types
typedef uint64_t PulseDeviceSize;
typedef uint32_t PulseFlags;

PULSE_DEFINE_NULLABLE_HANDLE(PulseBuffer);
PULSE_DEFINE_NULLABLE_HANDLE(PulseCommandList);
PULSE_DEFINE_NULLABLE_HANDLE(PulseComputePass);
PULSE_DEFINE_NULLABLE_HANDLE(PulseComputePipeline);
PULSE_DEFINE_NULLABLE_HANDLE(PulseCopyPass);
PULSE_DEFINE_NULLABLE_HANDLE(PulseDevice);
PULSE_DEFINE_NULLABLE_HANDLE(PulseFence);
PULSE_DEFINE_NULLABLE_HANDLE(PulseImage);

// Flags
typedef enum PulseBufferUsageBits
{
	PULSE_BUFFER_USAGE_TRANSFER_UPLOAD        = PULSE_BIT(1),
	PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD      = PULSE_BIT(2),
	PULSE_BUFFER_USAGE_TRANSFER_STORAGE_READ  = PULSE_BIT(3),
	PULSE_BUFFER_USAGE_TRANSFER_STORAGE_WRITE = PULSE_BIT(4),
} PulseShaderFormatBits;
typedef PulseFlags PulseBufferUsageFlags;

/**
 * With regards to compute storage usage, READ | WRITE means that you can have shader A
 * that only writes into the image and shader B that only reads from the image and
 * bind the same image to either shader respectively. SIMULTANEOUS means that you can
 * do reads and writes within the same shader or compute pass.
 */
typedef enum PulseImageUsageBits
{
	PULSE_IMAGE_USAGE_TRANSFER_STORAGE_READ                   = PULSE_BIT(1),
	PULSE_IMAGE_USAGE_TRANSFER_STORAGE_WRITE                  = PULSE_BIT(2),
	PULSE_IMAGE_USAGE_TRANSFER_STORAGE_SIMULTANEOUS_READWRITE = PULSE_BIT(3),
} PulseImageFormatBits;
typedef PulseFlags PulseImageUsageFlags;

typedef enum PulseShaderFormatsBits
{
	PULSE_SHADER_FORMAT_INVALID_BIT = PULSE_BIT(1),
	PULSE_SHADER_FORMAT_SPIRV_BIT   = PULSE_BIT(2),
	// More to come
} PulseShaderFormatsBits;
typedef PulseFlags PulseShaderFormatsFlags;

// Enums
typedef enum PulseImageType
{
	PULSE_IMAGE_TYPE_2D,
	PULSE_IMAGE_TYPE_2D_ARRAY,
	PULSE_IMAGE_TYPE_3D,
	PULSE_IMAGE_TYPE_CUBE,
	PULSE_IMAGE_TYPE_CUBE_ARRAY
} PulseImageType;

typedef enum PulseImageFormat
{
	PULSE_IMAGE_FORMAT_INVALID,
	// Unsigned Normalized Float Color Formats
	PULSE_IMAGE_FORMAT_A8_UNORM,
	PULSE_IMAGE_FORMAT_R8_UNORM,
	PULSE_IMAGE_FORMAT_R8G8_UNORM,
	PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM,
	PULSE_IMAGE_FORMAT_R16_UNORM,
	PULSE_IMAGE_FORMAT_R16G16_UNORM,
	PULSE_IMAGE_FORMAT_R16G16B16A16_UNORM,
	PULSE_IMAGE_FORMAT_R10G10B10A2_UNORM,
	PULSE_IMAGE_FORMAT_B5G6R5_UNORM,
	PULSE_IMAGE_FORMAT_B5G5R5A1_UNORM,
	PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM,
	PULSE_IMAGE_FORMAT_B8G8R8A8_UNORM,
	// Compressed Unsigned Normalized Float Color Formats
	PULSE_IMAGE_FORMAT_BC1_RGBA_UNORM,
	PULSE_IMAGE_FORMAT_BC2_RGBA_UNORM,
	PULSE_IMAGE_FORMAT_BC3_RGBA_UNORM,
	PULSE_IMAGE_FORMAT_BC4_R_UNORM,
	PULSE_IMAGE_FORMAT_BC5_RG_UNORM,
	PULSE_IMAGE_FORMAT_BC7_RGBA_UNORM,
	// Compressed Signed Float Color Formats
	PULSE_IMAGE_FORMAT_BC6H_RGB_FLOAT,
	// Compressed Unsigned Float Color Formats
	PULSE_IMAGE_FORMAT_BC6H_RGB_UFLOAT,
	// Signed Normalized Float Color Formats
	PULSE_IMAGE_FORMAT_R8_SNORM,
	PULSE_IMAGE_FORMAT_R8G8_SNORM,
	PULSE_IMAGE_FORMAT_R8G8B8A8_SNORM,
	PULSE_IMAGE_FORMAT_R16_SNORM,
	PULSE_IMAGE_FORMAT_R16G16_SNORM,
	PULSE_IMAGE_FORMAT_R16G16B16A16_SNORM,
	// Signed Float Color Formats
	PULSE_IMAGE_FORMAT_R16_FLOAT,
	PULSE_IMAGE_FORMAT_R16G16_FLOAT,
	PULSE_IMAGE_FORMAT_R16G16B16A16_FLOAT,
	PULSE_IMAGE_FORMAT_R32_FLOAT,
	PULSE_IMAGE_FORMAT_R32G32_FLOAT,
	PULSE_IMAGE_FORMAT_R32G32B32A32_FLOAT,
	// Unsigned Float Color Formats
	PULSE_IMAGE_FORMAT_R11G11B10_UFLOAT,
	// Unsigned Integer Color Formats
	PULSE_IMAGE_FORMAT_R8_UINT,
	PULSE_IMAGE_FORMAT_R8G8_UINT,
	PULSE_IMAGE_FORMAT_R8G8B8A8_UINT,
	PULSE_IMAGE_FORMAT_R16_UINT,
	PULSE_IMAGE_FORMAT_R16G16_UINT,
	PULSE_IMAGE_FORMAT_R16G16B16A16_UINT,
	PULSE_IMAGE_FORMAT_R32_UINT,
	PULSE_IMAGE_FORMAT_R32G32_UINT,
	PULSE_IMAGE_FORMAT_R32G32B32A32_UINT,
	// Signed Integer Color Formats
	PULSE_IMAGE_FORMAT_R8_INT,
	PULSE_IMAGE_FORMAT_R8G8_INT,
	PULSE_IMAGE_FORMAT_R8G8B8A8_INT,
	PULSE_IMAGE_FORMAT_R16_INT,
	PULSE_IMAGE_FORMAT_R16G16_INT,
	PULSE_IMAGE_FORMAT_R16G16B16A16_INT,
	PULSE_IMAGE_FORMAT_R32_INT,
	PULSE_IMAGE_FORMAT_R32G32_INT,
	PULSE_IMAGE_FORMAT_R32G32B32A32_INT,
	// SRGB Unsigned Normalized Color Formats
	PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM_SRGB,
	PULSE_IMAGE_FORMAT_B8G8R8A8_UNORM_SRGB,
	// Compressed SRGB Unsigned Normalized Color Formats
	PULSE_IMAGE_FORMAT_BC1_RGBA_UNORM_SRGB,
	PULSE_IMAGE_FORMAT_BC2_RGBA_UNORM_SRGB,
	PULSE_IMAGE_FORMAT_BC3_RGBA_UNORM_SRGB,
	PULSE_IMAGE_FORMAT_BC7_RGBA_UNORM_SRGB,
} PulseImageFormat;

// Structs

typedef struct PulseBufferCreateInfo
{
	PulseBufferUsageFlags usage;
	PulseDeviceSize size;
} PulseBufferCreateInfo;

typedef struct PulseBufferLocation
{
	PulseBuffer buffer;
	PulseDeviceSize offset;
} PulseBufferLocation;

typedef struct PulseBufferRegion
{
	PulseBuffer buffer;
	PulseDeviceSize offset;
	PulseDeviceSize size;
} PulseBufferRegion;

typedef struct PulseComputePipelineCreateInfo
{
	uint64_t code_size;
	const uint8_t* code;
	const char* entrypoint;
	PulseShaderFormatsFlags format;
	uint32_t num_samplers;
	uint32_t num_readonly_storage_images;
	uint32_t num_readonly_storage_buffers;
	uint32_t num_readwrite_storage_images;
	uint32_t num_readwrite_storage_buffers;
	uint32_t num_uniform_buffers;
	uint32_t threadcount_x;
	uint32_t threadcount_y;
	uint32_t threadcount_z;
} PulseComputePipelineCreateInfo;

typedef struct PulseIndirectDispatchCommand
{
	uint32_t groupcount_x;
	uint32_t groupcount_y;
	uint32_t groupcount_z;
} PulseIndirectDispatchCommand;

typedef struct PulseImageCreateInfo
{
	PulseImageType type;
	PulseImageFormat format;
	PulseImageUsageFlags usage;
	uint32_t width;
	uint32_t height;
	uint32_t layer_count_or_depth; // This value is treated as a layer count on 2D array textures, and as a depth value on 3D textures
} PulseImageCreateInfo;

typedef struct PulseImageLocation
{
	PulseImage image;
	uint32_t layer;
	uint32_t x;
	uint32_t y;
	uint32_t z;
} PulseImageLocation;

typedef struct PulseImageRegion
{
	PulseImage image;
	uint32_t layer;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} PulseImageRegion;

#ifdef __cplusplus
}
#endif

#endif // PULSE_H_
