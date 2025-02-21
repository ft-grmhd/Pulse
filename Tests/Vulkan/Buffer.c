#include "Common.h"

#include <string.h>
#include <unity/unity.h>
#include <Pulse.h>

void TestBufferCreation()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_WRITE;
	buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;
	buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_TRANSFER_UPLOAD;
	buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE;
	buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_WRITE | PULSE_BUFFER_USAGE_TRANSFER_UPLOAD;
	buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	buffer_create_info.size = 1024;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD | PULSE_BUFFER_USAGE_TRANSFER_UPLOAD;
	buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyBuffer(device, buffer);

	DISABLE_ERRORS;
		buffer_create_info.size = -1;
		buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ;
		buffer = PulseCreateBuffer(device, &buffer_create_info);
		TEST_ASSERT_EQUAL(buffer, PULSE_NULL_HANDLE);
		PulseGetLastErrorType(); // Just to clear the error code
		PulseDestroyBuffer(device, buffer);
	ENABLE_ERRORS;

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestBufferMapping()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	const unsigned char data[8] = { 0xA1, 0xFF, 0xDF, 0x17, 0x5B, 0xCC, 0x00, 0x36 };

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 8;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_TRANSFER_UPLOAD;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	{
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_NOT_NULL(ptr);
		memcpy(ptr, data, 8);
		PulseUnmapBuffer(buffer);
	}
	{
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_NOT_NULL(ptr);
		TEST_ASSERT_EQUAL(memcmp(ptr, data, 8), 0);
		PulseUnmapBuffer(buffer);
	}

	DISABLE_ERRORS;
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyBuffer(device, buffer);
	ENABLE_ERRORS;

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestBufferCopy()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	const unsigned char data[8] = { 0xA1, 0xFF, 0xDF, 0x17, 0x5B, 0xCC, 0x00, 0x36 };

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 8;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_TRANSFER_UPLOAD | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;
	PulseBuffer src_buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(src_buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	{
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(src_buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_NOT_NULL(ptr);
		memcpy(ptr, data, 8);
		PulseUnmapBuffer(src_buffer);
	}

	PulseBuffer dst_buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(dst_buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	{
		PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_TRANSFER_ONLY);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseFence fence = PulseCreateFence(device);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

		PulseBufferRegion src_region = { 0 };
		src_region.buffer = src_buffer;
		src_region.size = 8;

		PulseBufferRegion dst_region = { 0 };
		dst_region.buffer = dst_buffer;
		dst_region.size = 8;

		TEST_ASSERT_TRUE_MESSAGE(PulseCopyBufferToBuffer(cmd, &src_region, &dst_region), PulseVerbaliseErrorType(PulseGetLastErrorType()));

		TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

		PulseReleaseCommandList(device, cmd);
		PulseDestroyFence(device, fence);
	}

	{
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(dst_buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_NOT_NULL(ptr);
		TEST_ASSERT_EQUAL(memcmp(ptr, data, 8), 0);
		PulseUnmapBuffer(dst_buffer);
	}

	{
		PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_TRANSFER_ONLY);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseFence fence = PulseCreateFence(device);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

		PulseBufferRegion src_region = { 0 };
		src_region.buffer = src_buffer;
		src_region.size = 8;
		src_region.offset = 2;

		PulseBufferRegion dst_region = { 0 };
		dst_region.buffer = dst_buffer;
		dst_region.size = 12;

		DISABLE_ERRORS;
			TEST_ASSERT_FALSE_MESSAGE(PulseCopyBufferToBuffer(cmd, &src_region, &dst_region), PulseVerbaliseErrorType(PulseGetLastErrorType()));
		ENABLE_ERRORS;

		TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

		PulseReleaseCommandList(device, cmd);
		PulseDestroyFence(device, fence);
	}

	DISABLE_ERRORS;
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(src_buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyBuffer(device, src_buffer);
	ENABLE_ERRORS;

	PulseDestroyBuffer(device, dst_buffer);

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestBufferCopyImage()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	const unsigned char data[8] = { 0xA1, 0xFF, 0xDF, 0x17, 0x5B, 0xCC, 0x00, 0x36 };

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 8;
	buffer_create_info.usage = PULSE_BUFFER_USAGE_TRANSFER_UPLOAD;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	{
		void* ptr;
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_NOT_NULL(ptr);
		memcpy(ptr, data, 8);
		PulseUnmapBuffer(buffer);
	}

	PulseImageCreateInfo image_create_info = { 0 };
	image_create_info.type = PULSE_IMAGE_TYPE_2D;
	image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_WRITE;
	image_create_info.width = 8;
	image_create_info.height = 1;
	image_create_info.layer_count_or_depth = 1;
	PulseImage image = PulseCreateImage(device, &image_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	{
		PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_TRANSFER_ONLY);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseFence fence = PulseCreateFence(device);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

		PulseBufferRegion src_region = { 0 };
		src_region.buffer = buffer;
		src_region.size = 8;

		PulseImageRegion dst_region = { 0 };
		dst_region.image = image;
		dst_region.width = 8;
		dst_region.height = 1;
		dst_region.depth = 1;
		dst_region.x = 1;
		dst_region.y = 1;
		dst_region.z = 1;
		dst_region.layer = 1;

		TEST_ASSERT_TRUE_MESSAGE(PulseCopyBufferToImage(cmd, &src_region, &dst_region), PulseVerbaliseErrorType(PulseGetLastErrorType()));

		TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

		PulseReleaseCommandList(device, cmd);
		PulseDestroyFence(device, fence);
	}

	PulseDestroyImage(device, image);
	PulseDestroyBuffer(device, buffer);

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestBufferComputeWrite()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	const uint8_t shader_bytecode[] = {
		#include "Shaders/SimpleBufferWrite.spv.h"
	};

	PulseBufferCreateInfo buffer_create_info = { 0 };
	buffer_create_info.size = 256 * sizeof(int32_t);
	buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ | PULSE_BUFFER_USAGE_STORAGE_WRITE | PULSE_BUFFER_USAGE_TRANSFER_DOWNLOAD;
	PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePipeline pipeline;
	LoadComputePipeline(device, &pipeline, shader_bytecode, sizeof(shader_bytecode), 0, 0, 0, 1, 0);

	PulseFence fence = PulseCreateFence(device);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(fence, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseCommandList cmd = PulseRequestCommandList(device, PULSE_COMMAND_LIST_GENERAL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(cmd, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

	PulseComputePass pass = PulseBeginComputePass(cmd);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(pass, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseBindStorageBuffers(pass, 0, &buffer, 1);
		PulseBindComputePipeline(pass, pipeline);
		PulseDispatchComputations(pass, 32, 32, 1);
	PulseEndComputePass(pass);

	TEST_ASSERT_TRUE_MESSAGE(PulseSubmitCommandList(device, cmd, fence), PulseVerbaliseErrorType(PulseGetLastErrorType()));
	TEST_ASSERT_TRUE_MESSAGE(PulseWaitForFences(device, &fence, 1, true), PulseVerbaliseErrorType(PulseGetLastErrorType()));

	{
		void* ptr;
		uint32_t data[256];
		memset(data, 0xFF, 256 * sizeof(uint32_t));
		TEST_ASSERT_NOT_EQUAL_MESSAGE(PulseMapBuffer(buffer, &ptr), false, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		TEST_ASSERT_NOT_NULL(ptr);
		TEST_ASSERT_EQUAL(memcmp(ptr, data, 256 * sizeof(uint32_t)), 0);
		PulseUnmapBuffer(buffer);
	}

	PulseReleaseCommandList(device, cmd);
	PulseDestroyFence(device, fence);
	PulseDestroyBuffer(device, buffer);

	CleanupPipeline(device, pipeline);
	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestBufferDestruction()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	PulseDevice other_device;
	SetupDevice(backend, &other_device);

	{
		PulseBufferCreateInfo buffer_create_info = { 0 };
		buffer_create_info.size = 256;
		buffer_create_info.usage = PULSE_BUFFER_USAGE_STORAGE_READ;
		PulseBuffer buffer = PulseCreateBuffer(device, &buffer_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(buffer, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));

		DISABLE_ERRORS;
			RESET_ERRORS_CHECK;
			PulseDestroyBuffer(other_device, buffer);
			TEST_ASSERT_TRUE(HAS_RECIEVED_ERROR);
		ENABLE_ERRORS;
	}

	DISABLE_ERRORS;
		RESET_ERRORS_CHECK;
		CleanupDevice(device);
		TEST_ASSERT_TRUE(HAS_RECIEVED_ERROR);
	ENABLE_ERRORS;
	CleanupDevice(other_device);
	CleanupPulse(backend);
}

void TestBuffer()
{
	RUN_TEST(TestBufferCreation);
	RUN_TEST(TestBufferMapping);
	RUN_TEST(TestBufferCopy);
	RUN_TEST(TestBufferCopyImage);
	RUN_TEST(TestBufferComputeWrite);
	RUN_TEST(TestBufferDestruction);
}
