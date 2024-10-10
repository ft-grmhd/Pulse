#include <unity/unity.h>
#include <Pulse.h>

extern PulseBackend backend;

void TestDeviceSetup()
{
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(device, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseDestroyDevice(device);
}
