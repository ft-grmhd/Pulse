#include <Pulse.h>

#include <stdio.h>

int main(void)
{
	PulseBackend backend = PulseLoadBackend(PULSE_BACKEND_ANY, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_NO_DEBUG);
	if(backend == PULSE_NULL_HANDLE)
	{
		fprintf(stderr, "Error while loading Pulse: %s", PulseVerbaliseErrorType(PulseGetLastErrorType()));
		return 1;
	}
	PulseUnloadBackend(backend);
	puts("Successfully loaded Pulse !");
	return 0;
}
