#include <unity/unity.h>
#include <Pulse.h>

#include <stdbool.h>

extern void TestBackend();
extern void TestDevice();
extern void TestBuffer();
extern void TestImage();

int main(void)
{
	UNITY_BEGIN();
	TestBackend();
	TestDevice();
	TestBuffer();
	TestImage();
	return UNITY_END();
}
