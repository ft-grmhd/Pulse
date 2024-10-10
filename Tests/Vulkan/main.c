#include <unity/unity.h>
#include <Pulse.h>

#include <stdbool.h>

PulseBackend backend;

extern void TestDeviceSetup();

bool SetupPulse()
{
	backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_NO_DEBUG);
	if(backend == PULSE_NULL_HANDLE)
	{
		fprintf(stderr, "Fatal Error: could not load Pulse using Vulkan due to %s", PulseVerbaliseErrorType(PulseGetLastErrorType()));
		return false;
	}
	puts("Pulse loaded using Vulkan");
	return true;
}

int RunUnitTests()
{
	UNITY_BEGIN();
	RUN_TEST(TestDeviceSetup);
	return UNITY_END();
}

void UnloadPulse()
{
	PulseUnloadBackend(backend);
	puts("Pulse unloaded");
}

int main(void)
{
	SetupPulse();
	int result = RunUnitTests();
	UnloadPulse();
	return result;
}
