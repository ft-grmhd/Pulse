#include <Pulse.h>

#include <stdio.h>

int main(void)
{
	PulseDevice device = PulseCreateDevice(PULSE_BACKEND_ANY, PULSE_SHADER_FORMAT_SPIRV_BIT, PULSE_NO_DEBUG);
	if(device == PULSE_NULL_HANDLE)
	{
		fprintf(stderr, "Error while loading Pulse: %s", PulseVerbaliseErrorType(PulseGetLastErrorType()));
		return 1;
	}
	PulseDestroyDevice(device);
	return 0;
}
