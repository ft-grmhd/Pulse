// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_FENCE_H_
#define PULSE_SOFTWARE_FENCE_H_

#include <stdatomic.h>

#include <Pulse.h>
#include "Soft.h"

typedef struct SoftFence
{
	atomic_bool signal;
} SoftFence;

PulseFence SoftCreateFence(PulseDevice device);
void SoftDestroyFence(PulseDevice device, PulseFence fence);
bool SoftIsFenceReady(PulseDevice device, PulseFence fence);
bool SoftWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all);

#endif // PULSE_SOFTWARE_FENCE_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
