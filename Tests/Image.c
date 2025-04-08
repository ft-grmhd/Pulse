#include "Common.h"

#include <unity/unity.h>
#include <Pulse.h>

void TestImageCreation()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D_ARRAY;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}

	/**
	 * This test may crash some Nouveau NVK drivers (wtf ???).
	 * It seems to be comming exclusively from 3D read-only images
	 */
	#ifndef VULKAN_ENABLED
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_3D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}
	#endif

	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_CUBE;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 6;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_CUBE_ARRAY;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 12;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_WRITE;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_SIMULTANEOUS_READWRITE;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8_SNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		PulseDestroyImage(device, image);
	}

	DISABLE_ERRORS;
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 0;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_EQUAL(image, PULSE_NULL_HANDLE);
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = -1;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_EQUAL(image, PULSE_NULL_HANDLE);
		PulseDestroyImage(device, image);
	}
	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_CUBE;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_EQUAL(image, PULSE_NULL_HANDLE);
		PulseDestroyImage(device, image);
	}
	ENABLE_ERRORS;

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestImageDestruction()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	PulseDevice other_device;
	SetupDevice(backend, &other_device);

	{
		PulseImageCreateInfo image_create_info = { 0 };
		image_create_info.type = PULSE_IMAGE_TYPE_2D;
		image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
		image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
		image_create_info.width = 256;
		image_create_info.height = 256;
		image_create_info.layer_count_or_depth = 1;
		PulseImage image = PulseCreateImage(device, &image_create_info);
		TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
		DISABLE_ERRORS;
			RESET_ERRORS_CHECK;
			PulseDestroyImage(other_device, image);
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

void TestImage()
{
	RUN_TEST(TestImageCreation);
	RUN_TEST(TestImageDestruction);
}
