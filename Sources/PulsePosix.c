// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include "PulseInternal.h"
#include "PulsePosix.h"

#ifdef PULSE_PLAT_POSIX

PulseThreadID PulseGetThreadID()
{
	return (PulseThreadID)pthread_self();
}

#endif // PULSE_PLAT_POSIX
