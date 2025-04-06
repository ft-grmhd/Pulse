#include "Common.h"
#include <unity/unity.h>
#include <stdio.h>
#include <string.h>

bool errors_enabled = true;
bool has_recieved_error = false;

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	if(errors_enabled && severity == PULSE_DEBUG_MESSAGE_SEVERITY_ERROR)
	{
		fprintf(stderr, "%s\n", message);
		TEST_FAIL();
	}
	has_recieved_error = true;
}

#define LOG_MESSAGE_MAX_LENGTH 4096

void SetupPulse(PulseBackend* backend)
{
	#if defined(VULKAN_ENABLED)
		*backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_PARANOID_DEBUG);
	#elif defined(WEBGPU_ENABLED)
		*backend = PulseLoadBackend(PULSE_BACKEND_WEBGPU, PULSE_SHADER_FORMAT_WGSL_BIT, PULSE_PARANOID_DEBUG);
	#elif defined(OPENGL_ENABLED)
		*backend = PulseLoadBackend(PULSE_BACKEND_OPENGL, PULSE_SHADER_FORMAT_GLSL_BIT, PULSE_PARANOID_DEBUG);
	#elif defined(OPENGLES_ENABLED)
		*backend = PulseLoadBackend(PULSE_BACKEND_OPENGL_ES, PULSE_SHADER_FORMAT_GLSL_BIT, PULSE_PARANOID_DEBUG);
	#endif
	if(*backend == PULSE_NULL_HANDLE)
	{
		char complete_message[LOG_MESSAGE_MAX_LENGTH] = { 0 };
		snprintf(complete_message, LOG_MESSAGE_MAX_LENGTH, "Fatal Error: could not load Pulse backend due to %s", PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_FAIL_MESSAGE(complete_message);
		TEST_ABORT();
	}
	PulseSetDebugCallback(*backend, DebugCallBack);
}

void SetupDevice(PulseBackend backend, PulseDevice* device)
{
	*device = PulseCreateDevice(backend, NULL, 0);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(*device, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
}

void CleanupDevice(PulseDevice device)
{
	PulseDestroyDevice(device);
}

void CleanupPulse(PulseBackend backend)
{
	PulseUnloadBackend(backend);
}

void LoadComputePipeline(PulseDevice device, PulseComputePipeline* pipeline, const uint8_t* code, uint32_t code_size,
		uint32_t num_readonly_storage_images,
		uint32_t num_readonly_storage_buffers,
		uint32_t num_readwrite_storage_images,
		uint32_t num_readwrite_storage_buffers,
		uint32_t num_uniform_buffers)
{
	PulseComputePipelineCreateInfo info = { 0 };
	#if defined(WEBGPU_ENABLED)
		info.code_size = strlen(code);
	#else
		info.code_size = code_size;
	#endif
	info.code = code;
	info.entrypoint = "main";
	#if defined(VULKAN_ENABLED)
		info.format = PULSE_SHADER_FORMAT_SPIRV_BIT;
	#elif defined(WEBGPU_ENABLED)
		info.format = PULSE_SHADER_FORMAT_WGSL_BIT;
	#elif defined(OPENGL_ENABLED)
		info.format = PULSE_SHADER_FORMAT_GLSL_BIT;
	#elif defined(OPENGLES_ENABLED)
		info.format = PULSE_SHADER_FORMAT_GLSL_BIT;
	#endif
	info.num_readonly_storage_images = num_readonly_storage_images;
	info.num_readonly_storage_buffers = num_readonly_storage_buffers;
	info.num_readwrite_storage_buffers = num_readwrite_storage_buffers;
	info.num_readwrite_storage_images = num_readwrite_storage_images;
	info.num_uniform_buffers = num_uniform_buffers;

	*pipeline = PulseCreateComputePipeline(device, &info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(*pipeline, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
}

void CleanupPipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	PulseDestroyComputePipeline(device, pipeline);
}

void CopySameSizeBufferToBuffer(PulseDevice device, PulseBuffer src, PulseBuffer dst, PulseDeviceSize size)
{
	PulseFence fence = PulseCreateFence(device);
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_TRANSFER_ONLY);

	PulseBufferRegion src_region = { 0 };
	src_region.buffer = src;
	src_region.size = size;

	PulseBufferRegion dst_region = { 0 };
	dst_region.buffer = dst;
	dst_region.size = size;

	PulseCopyBufferToBuffer(cmd, &src_region, &dst_region);

	PulseSubmitCommandList(device, cmd, fence);
	PulseWaitForFences(device, &fence, 1, true);

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
}
