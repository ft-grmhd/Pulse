// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_INTERNAL_H_
#define PULSE_INTERNAL_H_

#include <Pulse.h>

#include "PulsePFNs.h"
#include "PulseDefs.h"
#include "PulseEnums.h"

typedef uint64_t PulseThreadID;

typedef struct PulseBackendHandler
{
	// PFNs
	PulseLoadBackendPFN PFN_LoadBackend;
	PulseUnloadBackendPFN PFN_UnloadBackend;
	PulseCreateDevicePFN PFN_CreateDevice;

	// Attributes
	PulseBackendFlags backend;
	PulseShaderFormatsFlags supported_shader_formats;
	void* driver_data;
	PulseDebugCallbackPFN PFN_UserDebugCallback;
	PulseDebugLevel debug_level;
} PulseBackendHandler;

typedef struct PulseBufferHandler
{
	PulseDevice device;
	void* driver_data;
	PulseBufferUsageFlags usage;
	PulseDeviceSize size;
	bool is_mapped;
} PulseBufferHandler;

typedef struct PulseCommandListHandler
{
	PulseDevice device;
	void* driver_data;
	PulseThreadID thread_id;
	PulseComputePipeline* compute_pipelines_bound;
	uint32_t compute_pipelines_bound_capacity;
	uint32_t compute_pipelines_bound_size;
	PulseCommandListState state;
	PulseCommandListUsage usage;
	bool is_available;
} PulseCommandListHandler;

typedef struct PulseComputePipelineHandler
{
	PulseCommandList cmd;
	void* driver_data;
} PulseComputePipelineHandler;

typedef struct PulseDeviceHandler
{
	// PFNs
	PulseDestroyDevicePFN PFN_DestroyDevice;
	PulseCreateComputePipelinePFN PFN_CreateComputePipeline;
	PulseDispatchComputePipelinePFN PFN_DispatchComputePipeline;
	PulseDestroyComputePipelinePFN PFN_DestroyComputePipeline;
	PulseCreateFencePFN PFN_CreateFence;
	PulseDestroyFencePFN PFN_DestroyFence;
	PulseIsFenceReadyPFN PFN_IsFenceReady;
	PulseWaitForFencesPFN PFN_WaitForFences;
	PulseRequestCommandListPFN PFN_RequestCommandList;
	PulseSubmitCommandListPFN PFN_SubmitCommandList;
	PulseReleaseCommandListPFN PFN_ReleaseCommandList;
	PulseCreateBufferPFN PFN_CreateBuffer;
	PulseMapBufferPFN PFN_MapBuffer;
	PulseUnmapBufferPFN PFN_UnmapBuffer;
	PulseDestroyBufferPFN PFN_DestroyBuffer;
	PulseCreateImagePFN PFN_CreateImage;
	PulseIsImageFormatValidPFN PFN_IsImageFormatValid;
	PulseDestroyImagePFN PFN_DestroyImage;

	// Attributes
	void* driver_data;
	PulseBackend backend;

	PulseBuffer* allocated_buffers;
	uint32_t allocated_buffers_size;
	uint32_t allocated_buffers_capacity;

	PulseImage* allocated_images;
	uint32_t allocated_images_size;
	uint32_t allocated_images_capacity;
} PulseDeviceHandler;

typedef struct PulseFenceHandler
{
	PulseCommandList cmd;
	void* driver_data;
} PulseFenceHandler;

typedef struct PulseImageHandler
{
	PulseDevice device;
	void* driver_data;
	PulseImageUsageFlags usage;
} PulseImageHandler;

PulseThreadID PulseGetThreadID();

void PulseSetInternalError(PulseErrorType error);

void PulseLogBackend(PulseBackend backend, PulseDebugMessageSeverity type, const char* message, const char* file, const char* function, int line, ...);

#define PulseLogError(backend, msg) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_ERROR, msg, __FILE__, __FUNCTION__, __LINE__)
#define PulseLogWarning(backend, msg) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_WARNING, msg, __FILE__, __FUNCTION__, __LINE__)
#define PulseLogInfo(backend, msg) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_INFO, msg, __FILE__, __FUNCTION__, __LINE__)

#define PulseLogErrorFmt(backend, msg, ...) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_ERROR, msg, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define PulseLogWarningFmt(backend, msg, ...) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_WARNING, msg, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define PulseLogInfoFmt(backend, msg, ...) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_INFO, msg, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define PULSE_MAX_STORAGE_TEXTURES_BOUND 8
#define PULSE_MAX_STORAGE_BUFFERS_BOUND  8
#define PULSE_MAX_UNIFORM_BUFFERS_BOUND  4
#define PULSE_MAX_WRITE_TEXTURES_BOUND   8
#define PULSE_MAX_WRITE_BUFFERS_BOUND    8

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	extern PulseBackendHandler VulkanDriver;
#endif // PULSE_ENABLE_VULKAN_BACKEND
#ifdef PULSE_ENABLE_D3D11_BACKEND
	extern PulseBackendHandler D3D11Driver;
#endif // PULSE_ENABLE_D3D11_BACKEND

#endif // PULSE_INTERNAL_H_
