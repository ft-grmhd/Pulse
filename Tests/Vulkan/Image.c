#include "Common.h"

#include <unity/unity.h>
#include <Pulse.h>

void TestImageCreation()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	PulseImageCreateInfo image_create_info = { 0 };
	image_create_info.type = PULSE_IMAGE_TYPE_2D;
	image_create_info.format = PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM;
	image_create_info.usage = PULSE_IMAGE_USAGE_STORAGE_READ;
	image_create_info.width = 256;
	image_create_info.height = 256;
	PulseImage image = PulseCreateImage(device, &image_create_info);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(image, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyImage(device, image);

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestImage()
{
	RUN_TEST(TestImageCreation);
}
