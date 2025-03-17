#include <signal.h>
#include <unity/unity.h>

#include <Pulse.h>

#ifdef BACKTRACE
	#include <backtrace.h>
	struct backtrace_state* state = NULL;
	static size_t trace_count = 0;

	void ErrorCallback(void* data, const char* msg, int errnum)
	{
		(void)data;
		fprintf(stderr, "Error in backtrace: %s (error %d)\n", msg, errnum);
	}

	int SymbolCallback(void* data, unsigned long pc, const char* filename, int lineno, const char* function)
	{
		(void)data;
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
#endif

extern void TestBackend();
extern void TestDevice();
extern void TestBuffer();
extern void TestImage();
extern void TestPipeline();

int main(void)
{
	#ifdef BACKTRACE
		state = backtrace_create_state(NULL, 1, ErrorCallback, NULL);
		signal(SIGSEGV, SignalHandler);
		signal(SIGABRT, SignalHandler);
	#endif

	UNITY_BEGIN();
	TestBackend();
	TestDevice();
	TestBuffer();
	TestImage();
	TestPipeline();
	return UNITY_END();
}
