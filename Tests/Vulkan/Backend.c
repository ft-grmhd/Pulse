#include "Common.h"

#include <unity/unity.h>
#include <Pulse.h>

void DumbDebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	(void)severity;
	(void)message;
}

void TestVulkanSupport()
{
	if(!PulseSupportsBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT))
	{
		TEST_MESSAGE("Vulkan is not supported");
		exit(1);
	}
}

void TestBackendSetup()
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_HIGH_DEBUG);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(backend, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseSetDebugCallback(backend, DumbDebugCallBack);
	PulseUnloadBackend(backend);
}

void TestBackendAnySetup()
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_ANY, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_HIGH_DEBUG);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(backend, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	TEST_ASSERT_EQUAL(PulseGetBackendType(backend), PULSE_BACKEND_VULKAN);
	PulseSetDebugCallback(backend, DumbDebugCallBack);
	PulseUnloadBackend(backend);
}

void TestWrongBackendSetup()
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_MSL_BIT, PULSE_HIGH_DEBUG);
	TEST_ASSERT_EQUAL(backend, PULSE_NULL_HANDLE);
	PulseSetDebugCallback(backend, DumbDebugCallBack);
	PulseUnloadBackend(backend);
}

void TestBackend()
{
	RUN_TEST(TestVulkanSupport);
	RUN_TEST(TestBackendSetup);
	RUN_TEST(TestBackendAnySetup);
	RUN_TEST(TestWrongBackendSetup);
}
