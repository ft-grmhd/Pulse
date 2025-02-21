# PulseGPU

[![Linux build](https://github.com/ft-grmhd/Pulse/actions/workflows/linux-build.yml/badge.svg)](https://github.com/ft-grmhd/Pulse/actions/workflows/linux-build.yml)
[![MacOS build](https://github.com/ft-grmhd/Pulse/actions/workflows/macos-build.yml/badge.svg)](https://github.com/ft-grmhd/Pulse/actions/workflows/macos-build.yml)
[![Msys2 build](https://github.com/ft-grmhd/Pulse/actions/workflows/msys2-build.yml/badge.svg)](https://github.com/ft-grmhd/Pulse/actions/workflows/msys2-build.yml)
[![Windows build](https://github.com/ft-grmhd/Pulse/actions/workflows/windows-build.yml/badge.svg)](https://github.com/ft-grmhd/Pulse/actions/workflows/windows-build.yml)

Pulse is a low level GPGPU library designed for highly intensive general GPU computations with high control over the hardware. It is built on top of Vulkan and a Metal support is in discussion.

```cpp
#include <Pulse.h>

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_NO_DEBUG);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);

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

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_WRITE;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);

	PulseFence fence = PulseCreateFence(device);
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);

	PulseComputePass pass = PulseBeginComputePass(cmd);
		PulseBindStorageBuffers(pass, 0, &buffer, 1);
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 32, 32, 1);
	PulseEndComputePass(pass);

	PulseSubmitCommandList(device, cmd, fence);
	PulseWaitForFences(device, &fence, 1, true);

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyComputePipeline(device, pipeline);

	PulseDestroyBuffer(device, buffer);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	return 0;
}
```
