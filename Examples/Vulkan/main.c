#include <Pulse.h>

#include <stdio.h>
#include <stdlib.h>

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message)
{
	if(severity == PULSE_DEBUG_MESSAGE_SEVERITY_ERROR)
	{
		fprintf(stderr, "Pulse Error: %s", message);
		exit(1);
	}
	else if(severity == PULSE_DEBUG_MESSAGE_SEVERITY_WARNING)
		fprintf(stderr, "Pulse Warning: %s", message);
	else
		printf("Pulse: %s", message);
}

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_VULKAN, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_NO_DEBUG);
	if(backend == PULSE_NULL_HANDLE)
	{
		fprintf(stderr, "Pulse: could not create backend");
		return 1;
	}
	PulseSetDebugCallback(backend, DebugCallBack);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);

	PulseDestroyDevice(device);
	PulseUnloadBackend(backend);
	puts("Successfully loaded Pulse using Vulkan !");
	return 0;
}
