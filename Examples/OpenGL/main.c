#include <Pulse.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WGSL_SOURCE(...) #__VA_ARGS__

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

int main(int ac, char** av)
{
	PulseBackendFlags backend_type = PULSE_BACKEND_OPENGL;
	if(ac >= 2 && strcmp(av[1], "--opengl-es") == 0)
		backend_type = PULSE_BACKEND_OPENGL_ES;
	PulseBackend backend = PulseLoadBackend(backend_type, PULSE_SHADER_FORMAT_GLSL_BIT, PULSE_HIGH_DEBUG);
	PulseSetDebugCallback(backend, DebugCallBack);

	PulseUnloadBackend(backend);
	puts("Successfully executed Pulse example using OpenGL !");
	return 0;
}
