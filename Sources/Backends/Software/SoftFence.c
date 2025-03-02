// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftFence.h"

PulseFence SoftCreateFence(PulseDevice device)
{
}

void SoftDestroyFence(PulseDevice device, PulseFence fence)
{
}

bool SoftIsFenceReady(PulseDevice device, PulseFence fence)
{
}

bool SoftWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all)
{
}
