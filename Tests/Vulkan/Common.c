#include "Common.h"
#include <unity/unity.h>

bool errors_enabled = true;
bool has_recieved_error = false;

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	if(errors_enabled && severity == PULSE_DEBUG_MESSAGE_SEVERITY_ERROR)
		TEST_FAIL_MESSAGE(message);
	has_recieved_error = true;
}

#define LOG_MESSAGE_MAX_LENGTH 4096

void SetupPulse(PulseBackend* backend)
{
	*backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_PARANOID_DEBUG);
	if(*backend == PULSE_NULL_HANDLE)
	{
		char complete_message[LOG_MESSAGE_MAX_LENGTH] = { 0 };
		snprintf(complete_message, LOG_MESSAGE_MAX_LENGTH, "Fatal Error: could not load Pulse using Vulkan due to %s", PulseVerbaliseErrorType(PulseGetLastErrorType()));
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
	info.code_size = code_size;
	info.code = code;
	info.entrypoint = "main";
	info.format = PULSE_SHADER_FORMAT_SPIRV_BIT;
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
