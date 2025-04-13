// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_INTERNAL_H_
#define PULSE_INTERNAL_H_

#include <Pulse.h>

#include "PulsePFNs.h"
#include "PulseDefs.h"
#include "PulseEnums.h"

#ifdef PULSE_COMPILER_MINGW
	#include <minwindef.h>
#endif

#ifdef PULSE_COMPILER_MSVC
	#include "MsvcAtomicsCompat.h"
#endif

#define PULSE_MAX_READ_TEXTURES_BOUND    8
#define PULSE_MAX_READ_BUFFERS_BOUND     8
#define PULSE_MAX_WRITE_TEXTURES_BOUND   8
#define PULSE_MAX_WRITE_BUFFERS_BOUND    8
#define PULSE_MAX_UNIFORM_BUFFERS_BOUND  8

typedef enum PulseInternalBufferUsageBits
{
	PULSE_INTERNAL_BUFFER_USAGE_UNIFORM_ACCESS = PULSE_BIT(31),
	PULSE_INTERNAL_BUFFER_USAGE_PURE_TRANSFER = PULSE_BIT(30),
} PulseInternalBufferUsageBits;

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
	PulseComputePass pass;
	void* driver_data;
	PulseThreadID thread_id;
	PulseCommandListState state;
	PulseCommandListUsage usage;
	bool is_available;
} PulseCommandListHandler;

typedef struct PulseComputePipelineHandler
{
	void* driver_data;
	uint32_t num_readonly_storage_images;
	uint32_t num_readonly_storage_buffers;
	uint32_t num_readwrite_storage_images;
	uint32_t num_readwrite_storage_buffers;
	uint32_t num_uniform_buffers;
} PulseComputePipelineHandler;

typedef struct PulseDeviceHandler
{
	// PFNs
	PulseDestroyDevicePFN PFN_DestroyDevice;
	PulseCreateComputePipelinePFN PFN_CreateComputePipeline;
	PulseDispatchComputationsPFN PFN_DispatchComputations;
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
	PulseCopyBufferToBufferPFN PFN_CopyBufferToBuffer;
	PulseCopyBufferToImageFN PFN_CopyBufferToImage;
	PulseDestroyBufferPFN PFN_DestroyBuffer;
	PulseCreateImagePFN PFN_CreateImage;
	PulseIsImageFormatValidPFN PFN_IsImageFormatValid;
	PulseCopyImageToBufferPFN PFN_CopyImageToBuffer;
	PulseBlitImagePFN PFN_BlitImage;
	PulseDestroyImagePFN PFN_DestroyImage;
	PulseBeginComputePassPFN PFN_BeginComputePass;
	PulseBindStorageBuffersPFN PFN_BindStorageBuffers;
	PulseBindUniformDataPFN PFN_BindUniformData;
	PulseBindStorageImagesPFN PFN_BindStorageImages;
	PulseBindComputePipelinePFN PFN_BindComputePipeline;
	PulseEndComputePassPFN PFN_EndComputePass;

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
	PulseImageType type;
	PulseImageFormat format;
	PulseImageUsageFlags usage;
	uint32_t width;
	uint32_t height;
	uint32_t layer_count_or_depth;
} PulseImageHandler;

typedef struct PulseComputePassHandler
{
	PulseBuffer readonly_storage_buffers[PULSE_MAX_READ_BUFFERS_BOUND];
	PulseBuffer readwrite_storage_buffers[PULSE_MAX_WRITE_BUFFERS_BOUND];

	PulseImage readonly_images[PULSE_MAX_READ_TEXTURES_BOUND];
	PulseImage readwrite_images[PULSE_MAX_WRITE_TEXTURES_BOUND];

	PulseBuffer uniform_buffers[PULSE_MAX_UNIFORM_BUFFERS_BOUND];

	PulseCommandList cmd;
	PulseComputePipeline current_pipeline;

	void* driver_data;

	PulseComputePipeline* compute_pipelines_bound;
	uint32_t compute_pipelines_bound_capacity;
	uint32_t compute_pipelines_bound_size;

	bool is_recording;
} PulseComputePassHandler;

PulseThreadID PulseGetThreadID();
void PulseSleep(int32_t ms);

#ifdef PULSE_PLAT_WINDOWS
	typedef const char* LPCSTR;
	typedef struct HINSTANCE__* HINSTANCE;
	typedef HINSTANCE HMODULE;
	#if defined(_MINWINDEF_)
		/* minwindef.h defines FARPROC, and attempting to redefine it may conflict with -Wstrict-prototypes */
	#elif defined(_WIN64)
		typedef __int64 (__stdcall* FARPROC)(void);
	#else
		typedef int (__stdcall* FARPROC)(void);
	#endif
#else
	#include <dlfcn.h>
#endif

#ifdef PULSE_PLAT_WINDOWS
	typedef HMODULE PulseLibModule;
#else
	typedef void* PulseLibModule;
#endif

typedef void(*PFN_PulseLibFunction)(void);

#define PULSE_NULL_LIB_MODULE PULSE_NULLPTR

PulseLibModule PulseLoadLibrary(const char* libpath); // Returns PULSE_NULL_LIB_MODULE in case of failure
PFN_PulseLibFunction PulseLoadSymbolFromLibModule(PulseLibModule module, const char* symbol);
void PulseUnloadLibrary(PulseLibModule module);

void PulseSetInternalError(PulseErrorType error);

uint32_t PulseHashString(const char* str);
uint32_t PulseHashCombine(uint32_t lhs, uint32_t rhs);

size_t PulseStrlcpy(char* dst, const char* src, size_t maxlen);
char* PulseStrtokR(char* str, const char* delim, char** saveptr);
void PulseTrimString(char* str);

void PulseLogBackend(PulseBackend backend, PulseDebugMessageSeverity type, const char* message, const char* file, const char* function, int line, ...);

#define PulseLogError(backend, msg) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_ERROR, msg, __FILE__, __FUNCTION__, __LINE__)
#define PulseLogWarning(backend, msg) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_WARNING, msg, __FILE__, __FUNCTION__, __LINE__)
#define PulseLogInfo(backend, msg) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_INFO, msg, __FILE__, __FUNCTION__, __LINE__)

#define PulseLogErrorFmt(backend, msg, ...) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_ERROR, msg, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define PulseLogWarningFmt(backend, msg, ...) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_WARNING, msg, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define PulseLogInfoFmt(backend, msg, ...) PulseLogBackend(backend, PULSE_DEBUG_MESSAGE_SEVERITY_INFO, msg, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef PULSE_ENABLE_VULKAN_BACKEND
	extern PulseBackendHandler VulkanDriver;
#endif // PULSE_ENABLE_VULKAN_BACKEND
#ifdef PULSE_ENABLE_METAL_BACKEND
	extern PulseBackendHandler MetalDriver;
#endif // PULSE_ENABLE_METAL_BACKEND
#ifdef PULSE_ENABLE_WEBGPU_BACKEND
	extern PulseBackendHandler WebGPUDriver;
#endif // PULSE_ENABLE_WEBGPU_BACKEND
#ifdef PULSE_ENABLE_SOFTWARE_BACKEND
	extern PulseBackendHandler SoftwareDriver;
#endif // PULSE_ENABLE_SOFTWARE_BACKEND
#ifdef PULSE_ENABLE_OPENGL_BACKEND
	extern PulseBackendHandler OpenGLDriver;
	extern PulseBackendHandler OpenGLESDriver;
#endif // PULSE_ENABLE_OPENGL_BACKEND
#ifdef PULSE_ENABLE_D3D11_BACKEND
	extern PulseBackendHandler D3D11Driver;
#endif // PULSE_ENABLE_D3D11_BACKEND

#endif // PULSE_INTERNAL_H_
