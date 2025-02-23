#include <Pulse.h>

#include <stdio.h>
#include <stdlib.h>

#define CHECK_PULSE_HANDLE_RETVAL(handle, retval) \
	if(handle == PULSE_NULL_HANDLE) \
	{ \
		fprintf(stderr, "Error: '" #handle "' %s\n", PulseVerbaliseErrorType(PulseGetLastErrorType())); \
		return retval; \
	} \

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	if(severity == PULSE_DEBUG_MESSAGE_SEVERITY_ERROR)
	{
		fprintf(stderr, "Pulse Error: %s\n", message);
		exit(1);
	}
	else if(severity == PULSE_DEBUG_MESSAGE_SEVERITY_WARNING)
		fprintf(stderr, "Pulse Warning: %s\n", message);
	else
		printf("Pulse: %s\n", message);
}

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_WEBGPU, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_HIGH_DEBUG);
	CHECK_PULSE_HANDLE_RETVAL(backend, 1);
	PulseSetDebugCallback(backend, DebugCallBack);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);
	CHECK_PULSE_HANDLE_RETVAL(device, 1);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	puts("Successfully executed Pulse example using WebGPU !");
	return 0;
}
