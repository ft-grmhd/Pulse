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

#ifdef WINTRACE
	#include <windows.h>
	#include <dbghelp.h>

	LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
	{
		fprintf(stderr, "Exception occurred!\n");
		PrintStackTrace();
		return EXCEPTION_EXECUTE_HANDLER;
	}

	void PrintStackTrace()
	{
		// Initialize symbols
		HANDLE process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);

		// Capture stack backtrace
		void* stack[62];
		USHORT frames = CaptureStackBackTrace(0, 62, stack, NULL);

		SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		for(USHORT i = 0; i < frames; i++)
		{
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
			fprintf(stderr, "[%d] %s - 0x%0llX\n", i, symbol->Name, symbol->Address);
		}

		free(symbol);
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

	#ifdef WINTRACE
		SetUnhandledExceptionFilter(ExceptionHandler);
	#endif

	UNITY_BEGIN();
	TestBackend();
	TestDevice();
	TestBuffer();
	TestImage();
	TestPipeline();
	return UNITY_END();
}
