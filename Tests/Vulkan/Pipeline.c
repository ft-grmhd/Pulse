#include "Common.h"

#include <unity/unity.h>
#include <Pulse.h>

void TestPipelineSetup()
{
	PulseBackend backend;
	SetupPulse(&backend);
	PulseDevice device;
	SetupDevice(backend, &device);

	CleanupDevice(device);
	CleanupPulse(backend);
}

void TestPipeline()
{
	RUN_TEST(TestPipelineSetup);
}
