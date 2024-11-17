#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#define RUN_TEST_AT_LINE(fn, line) \
{ \
	Unity.TestFile = __FILE__; \
	Unity.CurrentTestName = #fn; \
	Unity.CurrentTestLineNumber = line; \
	Unity.NumberOfTests++; \
	UNITY_CLR_DETAILS(); \
	UNITY_EXEC_TIME_START(); \
	if(TEST_PROTECT()) \
		fn(); \
	UNITY_EXEC_TIME_STOP(); \
	UnityConcludeTest(); \
}

#define RUN_TEST(fn) RUN_TEST_AT_LINE(fn, __LINE__);

#include <Pulse.h>

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message);
void SetupPulse(PulseBackend* backend);
void CleanupPulse(PulseBackend backend);

#endif
