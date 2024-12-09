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
	buffer_create_info.usage = PULSE_BUFFER_USAGE_UNIFORM_ACCESS;
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

void TestBuffer()
{
	RUN_TEST(TestBufferCreation);
	RUN_TEST(TestBufferMapping);
}
