#include <Pulse.h>

#include <stdio.h>
#include <stdlib.h>

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

#define BUFFER_SIZE (256 * sizeof(uint32_t))

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_D3D11, PULSE_SHADER_FORMAT_DXBC_BIT, PULSE_HIGH_DEBUG);
	PulseSetDebugCallback(backend, DebugCallBack);
	PulseDevice device = PulseCreateDevice(backend, NULL, 0);
	PulseUnloadBackend(backend);
	puts("Successfully executed Pulse example using D3D11 !");
	return 0;
}
