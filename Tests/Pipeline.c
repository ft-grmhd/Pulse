#include "Common.h"

#include <unity/unity.h>
#include <Pulse.h>

void TestPipelineSetup()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	#if defined(VULKAN_ENABLED)
		const uint8_t shader_bytecode[] = {
			#include "Shaders/Vulkan/Simple.spv.h"
		};
	#elif defined(WEBGPU_ENABLED)
		#define SHADER_NAME shader_bytecode
		#include "Shaders/WebGPU/Simple.wgsl.h"
	#endif

	PulseComputePipeline pipeline;
	LoadComputePipeline(device, &pipeline, shader_bytecode, sizeof(shader_bytecode), 0, 0, 0, 0, 0);

	PulseFence fence = PulseCreateFence(device);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePass pass = PulseBeginComputePass(cmd);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(pass, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 16, 1, 1);
	PulseEndComputePass(pass);

	TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
	TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);

	CleanupPipeline(device, pipeline);
	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestPipelineReadOnlyBindings()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	#if defined(VULKAN_ENABLED)
		const uint8_t shader_bytecode[] = {
			#include "Shaders/Vulkan/ReadOnlyBindings.spv.h"
		};
	#elif defined(WEBGPU_ENABLED)
		#define SHADER_NAME shader_bytecode
		#include "Shaders/WebGPU/ReadOnlyBindings.wgsl.h"
	#endif

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 256 * sizeof(int32_t);
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseImageCreateInfo image_create_info = { 0 };
	image_create_info.type = PULSE_IMAGE_TYPE_2D;
	image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
	image_create_info.width = 256;
	image_create_info.height = 256;
	image_create_info.layer_count_or_depth = 1;
	PulseImage image = PulseCreateImage(device, &image_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePipeline pipeline;
	LoadComputePipeline(device, &pipeline, shader_bytecode, sizeof(shader_bytecode), 1, 1, 0, 0, 0);

	PulseFence fence = PulseCreateFence(device);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePass pass = PulseBeginComputePass(cmd);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(pass, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseBindStorageImages(pass, &image, 1);
		PulseBindStorageBuffers(pass, &buffer, 1);
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 16, 1, 1);
	PulseEndComputePass(pass);

	TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
	TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyBuffer(device, buffer);
	PulseDestroyImage(device, image);

	CleanupPipeline(device, pipeline);
	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestPipelineWriteOnlyBindings()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	#if defined(VULKAN_ENABLED)
		const uint8_t shader_bytecode[] = {
			#include "Shaders/Vulkan/WriteOnlyBindings.spv.h"
		};
	#elif defined(WEBGPU_ENABLED)
		#define SHADER_NAME shader_bytecode
		#include "Shaders/WebGPU/WriteOnlyBindings.wgsl.h"
	#endif

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 256 * sizeof(int32_t);
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_WRITE;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseImageCreateInfo image_create_info = { 0 };
	image_create_info.type = PULSE_IMAGE_TYPE_2D;
	image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_WRITE;
	image_create_info.width = 256;
	image_create_info.height = 256;
	image_create_info.layer_count_or_depth = 1;
	PulseImage image = PulseCreateImage(device, &image_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePipeline pipeline;
	LoadComputePipeline(device, &pipeline, shader_bytecode, sizeof(shader_bytecode), 0, 0, 1, 1, 0);

	PulseFence fence = PulseCreateFence(device);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePass pass = PulseBeginComputePass(cmd);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(pass, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseBindStorageImages(pass, &image, 1);
		PulseBindStorageBuffers(pass, &buffer, 1);
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 16, 1, 1);
	PulseEndComputePass(pass);

	TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
	TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyBuffer(device, buffer);
	PulseDestroyImage(device, image);

	CleanupPipeline(device, pipeline);
	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestPipelineReadWriteBindings()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	#if defined(VULKAN_ENABLED)
		const uint8_t shader_bytecode[] = {
			#include "Shaders/Vulkan/ReadWriteBindings.spv.h"
		};
	#elif defined(WEBGPU_ENABLED)
		#define SHADER_NAME shader_bytecode
		#include "Shaders/WebGPU/ReadWriteBindings.wgsl.h"
	#endif

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 256 * sizeof(int32_t);
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ;
	PulseBuffer read_buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(read_buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_WRITE;
	PulseBuffer write_buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(write_buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseImageCreateInfo image_create_info = { 0 };
	image_create_info.type = PULSE_IMAGE_TYPE_2D;
	image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
	image_create_info.width = 256;
	image_create_info.height = 256;
	image_create_info.layer_count_or_depth = 1;
	PulseImage read_image = PulseCreateImage(device, &image_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(read_image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_WRITE;
	PulseImage write_image = PulseCreateImage(device, &image_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(write_image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePipeline pipeline;
	LoadComputePipeline(device, &pipeline, shader_bytecode, sizeof(shader_bytecode), 1, 1, 1, 1, 0);

	PulseFence fence = PulseCreateFence(device);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePass pass = PulseBeginComputePass(cmd);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(pass, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseBindStorageImages(pass, &read_image, 1);
		PulseBindStorageImages(pass, &write_image, 1);
		PulseBindStorageBuffers(pass, &read_buffer, 1);
		PulseBindStorageBuffers(pass, &write_buffer, 1);
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 16, 1, 1);
	PulseEndComputePass(pass);

	TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
	TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyBuffer(device, read_buffer);
	PulseDestroyBuffer(device, write_buffer);
	PulseDestroyImage(device, read_image);
	PulseDestroyImage(device, write_image);

	CleanupPipeline(device, pipeline);
	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestPipeline()
{
	RUN_TEST(TestPipelineSetup);
	RUN_TEST(TestPipelineReadOnlyBindings);
	RUN_TEST(TestPipelineWriteOnlyBindings);
	RUN_TEST(TestPipelineReadWriteBindings);
}
