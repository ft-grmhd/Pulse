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
#include <stdbool.h>

#include "PulseProfile.h"

#define PULSE_VERSION PULSE_MAKE_VERSION(0, 0, 1)

// Types
typedef uint64_t PulseDeviceSize;
typedef uint32_t PulseFlags;

PULSE_DEFINE_NULLABLE_HANDLE(PulseBackend);
PULSE_DEFINE_NULLABLE_HANDLE(PulseBuffer);
PULSE_DEFINE_NULLABLE_HANDLE(PulseCommandList);
PULSE_DEFINE_NULLABLE_HANDLE(PulseComputePipeline);
PULSE_DEFINE_NULLABLE_HANDLE(PulseDevice);
PULSE_DEFINE_NULLABLE_HANDLE(PulseFence);
PULSE_DEFINE_NULLABLE_HANDLE(PulseImage);

// Flags
typedef enum PulseBackendBits
{
	PULSE_BACKEND_INVALID = PULSE_BIT(1),
	PULSE_BACKEND_ANY = PULSE_BIT(2),
	PULSE_BACKEND_VULKAN = PULSE_BIT(3),
	PULSE_BACKEND_D3D11 = PULSE_BIT(4),
	// More to come
} PulseBackendBits;
typedef PulseFlags PulseBackendFlags;

typedef enum PulseBufferUsageBits
{
	PULSE_BUFFER_USAGE_TRANSFER_UPLOAD   = PULSE_BIT(1),
	PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD = PULSE_BIT(2),
	PULSE_BUFFER_USAGE_STORAGE_READ      = PULSE_BIT(3),
	PULSE_BUFFER_USAGE_STORAGE_WRITE     = PULSE_BIT(4),
	PULSE_BUFFER_USAGE_UNIFORM_ACCESS    = PULSE_BIT(5),
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
	PULSE_IMAGE_USAGE_STORAGE_READ                   = PULSE_BIT(1),
	PULSE_IMAGE_USAGE_STORAGE_WRITE                  = PULSE_BIT(2),
	PULSE_IMAGE_USAGE_STORAGE_SIMULTANEOUS_READWRITE = PULSE_BIT(3),
} PulseImageFormatBits;
typedef PulseFlags PulseImageUsageFlags;

typedef enum PulseShaderFormatsBits
{
	PULSE_SHADER_FORMAT_SPIRV_BIT   = PULSE_BIT(1), // Can be used by Vulkan | D3D11 backends
	PULSE_SHADER_FORMAT_DXBC_BIT    = PULSE_BIT(2), // Can be used by D3D11 backend only
	// More to come
} PulseShaderFormatsBits;
typedef PulseFlags PulseShaderFormatsFlags;

// Enums
typedef enum PulseCommandListUsage
{
	PULSE_COMMAND_LIST_GENERAL,
	PULSE_COMMAND_LIST_TRANSFER_ONLY
} PulseCommandListUsage;

typedef enum PulseDebugLevel
{
	PULSE_NO_DEBUG = 0,
	PULSE_LOW_DEBUG = 1,
	PULSE_HIGH_DEBUG = 2,
	PULSE_PARANOID_DEBUG = 3 // Causes every warning to be treated as error
} PulseDebugLevel;

typedef enum PulseDebugMessageSeverity
{
	PULSE_DEBUG_MESSAGE_SEVERITY_INFO,
	PULSE_DEBUG_MESSAGE_SEVERITY_WARNING,
	PULSE_DEBUG_MESSAGE_SEVERITY_ERROR
} PulseDebugMessageSeverity;

typedef enum PulseErrorType
{
	PULSE_ERROR_NONE,
	PULSE_ERROR_BACKENDS_CANDIDATES_SHADER_FORMAT_MISMATCH,
	PULSE_ERROR_INVALID_BACKEND,
	PULSE_ERROR_INITIALIZATION_FAILED,
	PULSE_ERROR_INVALID_HANDLE,
	PULSE_ERROR_CPU_ALLOCATION_FAILED,
	PULSE_ERROR_DEVICE_ALLOCATION_FAILED,
	PULSE_ERROR_DEVICE_LOST,
	PULSE_ERROR_INVALID_INTERNAL_POINTER,
	PULSE_ERROR_MAP_FAILED,
	PULSE_ERROR_INVALID_DEVICE,
} PulseErrorType;

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

	PULSE_IMAGE_FORMAT_MAX_ENUM // For internal use only
} PulseImageFormat;

// Structs
typedef struct PulseBufferCreateInfo
{
	PulseBufferUsageFlags usage;
	PulseDeviceSize size;
} PulseBufferCreateInfo;

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
	uint32_t num_readonly_storage_images;
	uint32_t num_readonly_storage_buffers;
	uint32_t num_readwrite_storage_images;
	uint32_t num_readwrite_storage_buffers;
	uint32_t num_uniform_buffers;
} PulseComputePipelineCreateInfo;

typedef struct PulseImageCreateInfo
{
	PulseImageType type;
	PulseImageFormat format;
	PulseImageUsageFlags usage;
	uint32_t width;
	uint32_t height;
	uint32_t layer_count_or_depth; // This value is treated as a layer count on 2D array images, and as a depth value on 3D images
} PulseImageCreateInfo;

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

// Functions
typedef void (*PulseDebugCallbackPFN)(PulseDebugMessageSeverity, const char*);

PULSE_API bool PulseSupportsBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used);

PULSE_API PulseBackend PulseLoadBackend(PulseBackendFlags backend_candidates, PulseShaderFormatsFlags shader_formats_used, PulseDebugLevel debug_level);
PULSE_API PulseBackendFlags PulseGetBackendType(PulseBackend backend);
PULSE_API void PulseSetDebugCallback(PulseBackend backend, PulseDebugCallbackPFN callback);
PULSE_API void PulseUnloadBackend(PulseBackend backend);

PULSE_API PulseDevice PulseCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count);
PULSE_API PulseBackendBits PulseGetBackendInUseByDevice(PulseDevice device);
PULSE_API bool PulseDeviceSupportsShaderFormats(PulseDevice device, PulseShaderFormatsFlags shader_formats_used);
PULSE_API void PulseDestroyDevice(PulseDevice device);

PULSE_API PulseBuffer PulseCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos);
PULSE_API bool PulseMapBuffer(PulseBuffer buffer, void** data);
PULSE_API void PulseUnmapBuffer(PulseBuffer buffer);
PULSE_API bool PulseCopyBufferToBuffer(const PulseBufferRegion* src, const PulseBufferRegion* dst);
PULSE_API bool PulseCopyBufferToImage(PulseBuffer src, const PulseBufferRegion* dst);
PULSE_API void PulseDestroyBuffer(PulseDevice device, PulseBuffer buffer);

PULSE_API PulseImage PulseCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos);
PULSE_API bool PulseIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage);
PULSE_API bool PulseCopyImageToBuffer(const PulseImageRegion* src, const PulseBufferRegion* dst);
PULSE_API bool PulseBlitImage(const PulseImageRegion* src, const PulseImageRegion* dst);
PULSE_API void PulseDestroyImage(PulseDevice device, PulseImage image);

PULSE_API PulseCommandList PulseRequestCommandList(PulseDevice device, PulseCommandListUsage usage);
PULSE_API bool PulseSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
PULSE_API void PulseReleaseCommandList(PulseDevice device, PulseCommandList cmd);

PULSE_API PulseFence PulseCreateFence(PulseDevice device);
PULSE_API void PulseDestroyFence(PulseDevice device, PulseFence fence);
PULSE_API bool PulseIsFenceReady(PulseDevice device, PulseFence fence);
PULSE_API bool PulseWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all);

PULSE_API PulseComputePipeline PulseCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info);
PULSE_API void PulseDispatchComputePipeline(PulseComputePipeline pipeline, PulseCommandList cmd, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);
PULSE_API void PulseDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline);

PULSE_API PulseErrorType PulseGetLastErrorType(); // /!\ Warning /!\ Call to this function resets the internal last error variable
PULSE_API const char* PulseVerbaliseErrorType(PulseErrorType error);

#ifdef __cplusplus
}
#endif

#endif // PULSE_H_
