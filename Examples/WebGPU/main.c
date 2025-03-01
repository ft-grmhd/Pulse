#include <Pulse.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WGSL_SOURCE(...) #__VA_ARGS__

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

#define BUFFER_SIZE (256 * sizeof(uint32_t))

const char* wgsl_source = WGSL_SOURCE(
	@compute @workgroup_size(32, 32, 1)
	fn main(@builtin(global_invocation_id) grid: vec3u)
	{
	}
);

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_WEBGPU, PULSE_SHADER_FORMAT_WGSL_BIT, PULSE_HIGH_DEBUG);
	PulseSetDebugCallback(backend, DebugCallBack);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = BUFFER_SIZE;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);

	// GPU computations
	{
		PulseComputePipelineCreateInfo info = { 0 };
		info.code_size = strlen(wgsl_source);
		info.code = (const uint8_t*)wgsl_source;
		info.entrypoint = "main";
		info.format = PULSE_SHADER_FORMAT_WGSL_BIT;
		info.num_readwrite_storage_buffers = 1;
		PulseComputePipeline pipeline = PulseCreateComputePipeline(device, &info);

		PulseFence fence = PulseCreateFence(device);
		PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);

		PulseComputePass pass = PulseBeginComputePass(cmd);
		//	PulseBindStorageBuffers(pass, &buffer, 1);
			PulseBindComputePipeline(pass, pipeline);
			PulseDispatchComputations(pass, 32, 32, 1);
		PulseEndComputePass(pass);

		PulseSubmitCommandList(device, cmd, fence);
		PulseWaitForFences(device, &fence, 1, true);

		PulseReleaseCommandList(device, cmd);
		PulseDestroyFence(device, fence);
		PulseDestroyComputePipeline(device, pipeline);
	}

	// Get result and read it on CPU
	{
		PulseBufferCreateInfo staging_buffer_create_info = { 0 };
		staging_buffer_create_info.size = BUFFER_SIZE;
		staging_buffer_create_info.usage = PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;
		PulseBuffer staging_buffer = PulseCreateBuffer(device, &staging_buffer_create_info);

		PulseFence fence = PulseCreateFence(device);
		PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_TRANSFER_ONLY);

		PulseBufferRegion src_region = { 0 };
		src_region.buffer = buffer;
		src_region.size = BUFFER_SIZE;

		PulseBufferRegion dst_region = { 0 };
		dst_region.buffer = staging_buffer;
		dst_region.size = BUFFER_SIZE;

		PulseCopyBufferToBuffer(cmd, &src_region, &dst_region);

		PulseSubmitCommandList(device, cmd, fence);
		PulseWaitForFences(device, &fence, 1, true);

		void* ptr;
		PulseMapBuffer(staging_buffer, PULSE_MAP_READ, &ptr);
		for(uint32_t i = 0; i < BUFFER_SIZE / sizeof(uint32_t); i++)
			printf("%d, ", ((int32_t*)ptr)[i]);
		puts("");
		PulseUnmapBuffer(staging_buffer);

		PulseDestroyBuffer(device, staging_buffer);

		PulseReleaseCommandList(device, cmd);
		PulseDestroyFence(device, fence);
	}

	PulseDestroyBuffer(device, buffer);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	puts("Successfully executed Pulse example using WebGPU !");
	return 0;
}
