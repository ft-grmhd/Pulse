// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "PulseInternal.h"

#include <tinycthread.h>

PulseThreadID PulseGetThreadID()
{
	return (PulseThreadID)thrd_current();
}

void PulseSleep(int32_t ms)
{
	if(ms <= 0)
		return;
	thrd_sleep(&(struct timespec){ .tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000 }, PULSE_NULLPTR);
}
