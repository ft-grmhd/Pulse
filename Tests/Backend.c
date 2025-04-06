#include "Common.h"

#include <unity/unity.h>
#include <Pulse.h>

void DumbDebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	(void)severity;
	(void)message;
}

void TestBackendSupport()
{
	#if defined(VULKAN_ENABLED)
		if(!PulseSupportsBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT))
	#elif defined(WEBGPU_ENABLED)
		if(!PulseSupportsBackend(PULSE_BACKEND_WEBGPU, PULSE_SHADER_FORMAT_WGSL_BIT))
	#elif defined(OPENGL_ENABLED)
		if(!PulseSupportsBackend(PULSE_BACKEND_OPENGL, PULSE_SHADER_FORMAT_GLSL_BIT))
	#elif defined(OPENGLES_ENABLED)
		if(!PulseSupportsBackend(PULSE_BACKEND_OPENGL_ES, PULSE_SHADER_FORMAT_GLSL_BIT))
	#endif
	{
		TEST_MESSAGE("Backend is not supported");
		exit(0);
	}
}

void TestBackendSetup()
{
	#if defined(VULKAN_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_HIGH_DEBUG);
	#elif defined(WEBGPU_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_WEBGPU, PULSE_SHADER_FORMAT_WGSL_BIT, PULSE_HIGH_DEBUG);
	#elif defined(OPENGL_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_OPENGL, PULSE_SHADER_FORMAT_GLSL_BIT, PULSE_HIGH_DEBUG);
	#elif defined(OPENGLES_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_OPENGL_ES, PULSE_SHADER_FORMAT_GLSL_BIT, PULSE_HIGH_DEBUG);
	#endif
	TEST_ASSERT_NOT_EQUAL_MESSAGE(backend, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	PulseSetDebugCallback(backend, DumbDebugCallBack);
	PulseUnloadBackend(backend);
}

void TestBackendAnySetup()
{
	#if defined(VULKAN_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_ANY, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_HIGH_DEBUG);
	#elif defined(WEBGPU_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_ANY, PULSE_SHADER_FORMAT_WGSL_BIT, PULSE_HIGH_DEBUG);
	#elif defined(OPENGL_ENABLED) || defined(OPENGLES_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_ANY, PULSE_SHADER_FORMAT_GLSL_BIT, PULSE_HIGH_DEBUG);
	#endif
	TEST_ASSERT_NOT_EQUAL_MESSAGE(backend, PULSE_NULL_HANDLE, PulseVerbaliseErrorType(PulseGetLastErrorType()));
	#if defined(VULKAN_ENABLED)
		TEST_ASSERT_EQUAL(PulseGetBackendType(backend), PULSE_BACKEND_VULKAN);
	#elif defined(WEBGPU_ENABLED)
		TEST_ASSERT_EQUAL(PulseGetBackendType(backend), PULSE_BACKEND_WEBGPU);
	#elif defined(OPENGL_ENABLED) || defined(OPENGLES_ENABLED)
		PulseBackendFlags backend_type = PulseGetBackendType(backend);
		if(backend_type != PULSE_BACKEND_OPENGL && backend_type != PULSE_BACKEND_OPENGL_ES)
			TEST_FAIL();
	#endif
	PulseSetDebugCallback(backend, DumbDebugCallBack);
	PulseUnloadBackend(backend);
}

void TestWrongBackendSetup()
{
	#if defined(VULKAN_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_MSL_BIT, PULSE_HIGH_DEBUG);
	#elif defined(WEBGPU_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_WEBGPU, PULSE_SHADER_FORMAT_MSL_BIT, PULSE_HIGH_DEBUG);
	#elif defined(OPENGL_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_OPENGL, PULSE_SHADER_FORMAT_MSL_BIT, PULSE_HIGH_DEBUG);
	#elif defined(OPENGLES_ENABLED)
		PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_OPENGL_ES, PULSE_SHADER_FORMAT_MSL_BIT, PULSE_HIGH_DEBUG);
	#endif
	TEST_ASSERT_EQUAL(backend, PULSE_NULL_HANDLE);
	PulseSetDebugCallback(backend, DumbDebugCallBack);
	PulseUnloadBackend(backend);
}

void TestBackend()
{
	RUN_TEST(TestBackendSupport);
	RUN_TEST(TestBackendSetup);
	RUN_TEST(TestBackendAnySetup);
	RUN_TEST(TestWrongBackendSetup);
}
