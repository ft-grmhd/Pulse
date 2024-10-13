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
} PulseBackendHandler;

typedef struct PulseBufferHandler
{
	void* driver_data;
} PulseBufferHandler;

typedef struct PulseCommandListHandler
{
	PulseDevice device;
	void* driver_data;
	PulseCommandListState state;
	bool is_compute_pipeline_bound;
} PulseCommandListHandler;

typedef struct PulseComputePassHandler
{
	void* driver_data;
} PulseComputePassHandler;

typedef struct PulseComputePipelineHandler
{
	void* driver_data;
} PulseComputePipelineHandler;

typedef struct PulseDeviceHandler
{
	// PFNs
	PulseDestroyDevicePFN PFN_DestroyDevice;
	PulseCreateComputePipelinePFN PFN_CreateComputePipeline;
	PulseBindComputePipelinePFN PFN_BindComputePipeline;
	PulseDestroyComputePipelinePFN PFN_DestroyComputePipeline;
	PulseCreateFencePFN PFN_CreateFence;
	PulseDestroyFencePFN PFN_DestroyFence;
	PulseIsFenceReadyPFN PFN_IsFenceReady;
	PulseWaitForFencesPFN PFN_WaitForFences;

	// Attributes
	void* driver_data;
	PulseBackend backend;
} PulseDeviceHandler;

typedef struct PulseFenceHandler
{
	void* driver_data;
} PulseFenceHandler;

typedef struct PulseGeneralPassHandler
{
	void* driver_data;
} PulseGeneralPassHandler;

typedef struct PulseImageHandler
{
	void* driver_data;
} PulseImageHandler;

PulseThreadID PulseGetThreadID();

void PulseLogErrorBackend(PulseBackend backend, PulseErrorType error, const char* file, const char* function, int line);
void PulseLogWarningBackend(PulseBackend backend, PulseWarningType warning, const char* file, const char* function, int line);
void PulseLogInfoBackend(PulseBackend backend, const char* message, const char* file, const char* function, int line);

#define PulseLogError(backend, type) PulseSetInternalErrorBackend(backend, type, __FILE__, __FUNCTION__, __LINE__)
#define PulseLogWarning(backend, type) PulseSetInternalErrorBackend(backend, type, __FILE__, __FUNCTION__, __LINE__)
#define PulseLogInfo(backend, msg) PulseSetInternalErrorBackend(backend, msg, __FILE__, __FUNCTION__, __LINE__)

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	extern PulseBackendHandler VulkanDriver;
#endif // PULSE_ENABLE_VULKAN_BACKEND
#ifdef PULSE_ENABLE_D3D11_BACKEND
	extern PulseBackendHandler D3D11Driver;
#endif // PULSE_ENABLE_D3D11_BACKEND

#endif // PULSE_INTERNAL_H_
