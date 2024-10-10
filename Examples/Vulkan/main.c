#include <Pulse.h>

#include <stdio.h>

#define CHECK_PULSE_HANDLE_RETVAL(handle, retval) \
	if(handle == PULSE_NULL_HANDLE) \
	{ \
		fprintf(stderr, "Error: %s", PulseVerbaliseErrorType(PulseGetLastErrorType())); \
		return retval; \
	} \

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_NO_DEBUG);
	CHECK_PULSE_HANDLE_RETVAL(backend, 1);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);
	CHECK_PULSE_HANDLE_RETVAL(device, 1);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	puts("Successfully loaded Pulse using Vulkan !");
	return 0;
}
