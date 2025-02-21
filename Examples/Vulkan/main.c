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

	PulseComputePipelineCreateInfo info = { 0 };
	info.code_size = sizeof(shader_bytecode);
	info.code = shader_bytecode;
	info.entrypoint = "main";
	info.format = PULSE_SHADER_FORMAT_SPIRV_BIT;
	info.num_readwrite_storage_buffers = 1;

	PulseComputePipeline pipeline = PulseCreateComputePipeline(device, &info);
	CHECK_PULSE_HANDLE_RETVAL(pipeline, 1);

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 256 * sizeof(uint32_t);
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;

	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	CHECK_PULSE_HANDLE_RETVAL(buffer, 1);

	PulseFence fence = PulseCreateFence(device);
	CHECK_PULSE_HANDLE_RETVAL(fence, 1);
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	CHECK_PULSE_HANDLE_RETVAL(cmd, 1);

	PulseComputePass pass = PulseBeginComputePass(cmd);
	CHECK_PULSE_HANDLE_RETVAL(pass, 1);
		PulseBindStorageBuffers(pass, &buffer, 1);
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 32, 32, 1);
	PulseEndComputePass(pass);

	if(!PulseSubmitCommandList(device, cmd, fence))
		fprintf(stderr, "Could not submit command list, %s\n", PulseVerbaliseErrorType(PulseGetLastErrorType()));
	if(!PulseWaitForFences(device, &fence, 1, true))
		fprintf(stderr, "Could not wait for fences, %s\n", PulseVerbaliseErrorType(PulseGetLastErrorType()));

	void* ptr;
	PulseMapBuffer(buffer, &ptr);
	for(uint32_t i = 0; i < 256; i++)
		printf("%d, ", ((int32_t*)ptr)[i]);
	puts("");
	PulseUnmapBuffer(buffer);

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyComputePipeline(device, pipeline);

	PulseDestroyBuffer(device, buffer);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	puts("Successfully executed Pulse example using Vulkan !");
	return 0;
}
