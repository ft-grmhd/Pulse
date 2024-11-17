#include <unity/unity.h>
#include <Pulse.h>

#include <stdbool.h>

extern void TestBackend();
extern void TestDevice();

int main(void)
{
	UNITY_BEGIN();
	TestBackend();
	TestDevice();
	return UNITY_END();
}
