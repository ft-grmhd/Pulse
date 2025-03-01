#include <signal.h>
#include <backtrace.h>
#include <unity/unity.h>

#include <Pulse.h>

struct backtrace_state* state = NULL;
static size_t trace_count = 0;

void ErrorCallback(void* data, const char* msg, int errnum)
{
	fprintf(stderr, "Error in backtrace: %s (error %d)\n", msg, errnum);
}

int SymbolCallback(void* data, unsigned long pc, const char* filename, int lineno, const char* function)
{
	if(filename && function)
		fprintf(stderr, "\t#%zu\033[1;34m 0x%lx\033[1;0m in\033[1;33m %s() from %s:%d\033[1;0m\n", trace_count, pc, function, filename, lineno);
	else if(function)
		fprintf(stderr, "\t#%zu\033[1;34m 0x%lx\033[1;0m in\033[1;33m %s() from unknown:unknown\033[1;0m\n", trace_count, pc, function);
	else
		fprintf(stderr, "\t#%zu\033[1;34m 0x%lx\033[1;0m in\033[1;33m ??\()\033[1;0m\n", trace_count, pc);
	trace_count++;
	return 0;
}

void SignalHandler(int sig)
{
	fprintf(stderr, "\n==========================================================\nFatal error: %s.\n<Begin of stack trace>\n", sig == SIGSEGV ? "segmentation fault" : "aborted");
	trace_count = 0;
	backtrace_full(state, 0, SymbolCallback, ErrorCallback, NULL);
	fprintf(stderr, "<End of stack trace>\n==========================================================\n");
	exit(1);
}

extern void TestBackend();
extern void TestDevice();
extern void TestBuffer();
extern void TestImage();
extern void TestPipeline();

int main(void)
{
	state = backtrace_create_state(NULL, 1, ErrorCallback, NULL);
	signal(SIGSEGV, SignalHandler);
    signal(SIGABRT, SignalHandler);

	UNITY_BEGIN();
	TestBackend();
	TestDevice();
	TestBuffer();
	TestImage();
	TestPipeline();
	return UNITY_END();
}

/*
#0  0x00007ffff79ad624 in ?? () from /usr/lib/libc.so.6
#1  0x00007ffff7953ba0 in raise () from /usr/lib/libc.so.6
#2  0x000055555560ec86 in TestPipelineReadOnlyBindings () at Tests/Vulkan/Pipeline.c:54
#3  0x000055555560f682 in TestPipeline () at Tests/Vulkan/Pipeline.c:228
#4  0x000055555560ea01 in main () at Tests/Vulkan/main.c:46
*/
