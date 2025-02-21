#include <unity/unity.h>
#include <Pulse.h>

#include <stdbool.h>

extern void TestBackend();
extern void TestDevice();
extern void TestBuffer();
extern void TestImage();
extern void TestPipeline();

int main(void)
{
	UNITY_BEGIN();
	TestBackend();
	TestDevice();
	TestBuffer();
	TestImage();
	TestPipeline();
	return UNITY_END();
}
