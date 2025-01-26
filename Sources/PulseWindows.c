// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "PulseInternal.h"
#include "PulseWindows.h"

#ifdef PULSE_PLAT_WINDOWS

PulseThreadID PulseGetThreadID()
{
	return (PulseThreadID)GetCurrentThreadId();
}

#endif // PULSE_PLAT_WINDOWS
