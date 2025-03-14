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

extern bool errors_enabled;
#define DISABLE_ERRORS errors_enabled = false
#define ENABLE_ERRORS errors_enabled = true

extern bool has_recieved_error;
#define RESET_ERRORS_CHECK has_recieved_error = false
#define HAS_RECIEVED_ERROR ((bool)(has_recieved_error == true))

void DebugCallBack(PulseDebugMessageSeverity severity, const char* message);
void SetupPulse(PulseBackend* backend);
void SetupDevice(PulseBackend backend, PulseDevice* device);
void CleanupDevice(PulseDevice device);
void CleanupPulse(PulseBackend backend);
void LoadComputePipeline(PulseDevice device, PulseComputePipeline* pipeline, const uint8_t* code, uint32_t code_size,
		uint32_t num_readonly_storage_images,
		uint32_t num_readonly_storage_buffers,
		uint32_t num_readwrite_storage_images,
		uint32_t num_readwrite_storage_buffers,
		uint32_t num_uniform_buffers);
void CleanupPipeline(PulseDevice device, PulseComputePipeline pipeline);
void CopySameSizeBufferToBuffer(PulseDevice device, PulseBuffer src, PulseBuffer dst, PulseDeviceSize size);

#endif
