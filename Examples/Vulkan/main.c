#include <Pulse.h>

#include <stdio.h>
#include <stdlib.h>

#define CHECK_PULSE_HANDLE_RETVAL(handle, retval) \
	if(handle == PULSE_NULL_HANDLE) \
	{ \
		fprintf(stderr, "Error: %s", PulseVerbaliseErrorType(PulseGetLastErrorType())); \
		return retval; \
	} \

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	if(severity == PULSE_DEBUG_MESSAGE_SEVERITY_ERROR)
	{
		fprintf(stderr, "Pulse Error: %s\n", message);
		exit(1);
	}
	else if(severity == PULSE_DEBUG_MESSAGE_SEVERITY_WARNING)
		fprintf(stderr, "Pulse Warning: %s\n", message);
	else
		printf("Pulse: %s\n", message);
}

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_HIGH_DEBUG);
	CHECK_PULSE_HANDLE_RETVAL(backend, 1);
	PulseSetDebugCallback(backend, DebugCallBack);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);
	CHECK_PULSE_HANDLE_RETVAL(device, 1);

	const uint8_t shader_bytecode[] = {
		#include "shader.spv.h"
	};

	PulseComputePipelineCreateInfo info = {};
	info.code_size = sizeof(shader_bytecode);
	info.code = shader_bytecode;
	info.entrypoint = "main";
	info.format = PULSE_SHADER_FORMAT_SPIRV_BIT;

	PulseComputePipeline pipeline = PulseCreateComputePipeline(device, &info);
	CHECK_PULSE_HANDLE_RETVAL(pipeline, 1);

	PulseBufferCreateInfo buffer_create_info = {};
	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;

	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	CHECK_PULSE_HANDLE_RETVAL(buffer, 1);

	PulseFence fence = PulseCreateFence(device);
	CHECK_PULSE_HANDLE_RETVAL(fence, 1);
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	CHECK_PULSE_HANDLE_RETVAL(cmd, 1);

	PulseDispatchComputePipeline(pipeline, cmd, 8, 8, 8);

	if(!PulseSubmitCommandList(device, cmd, fence))
		fprintf(stderr, "Could not submit command list, %s\n", PulseVerbaliseErrorType(PulseGetLastErrorType()));
	if(!PulseWaitForFences(device, &fence, 1, true))
		fprintf(stderr, "Could not wait for fences, %s\n", PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyBuffer(device, buffer);
	PulseDestroyComputePipeline(device, pipeline);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	puts("Successfully loaded Pulse using Vulkan !");
	return 0;
}
