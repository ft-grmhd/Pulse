// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_COMMAND_LIST_H_
#define PULSE_SOFTWARE_COMMAND_LIST_H_

#include "Soft.h"

typedef struct SoftCommandList
{
} SoftCommandList;

PulseCommandList SoftRequestCommandList(PulseDevice device, PulseCommandListUsage usage);
bool SoftSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence);
void SoftReleaseCommandList(PulseDevice device, PulseCommandList cmd);

#endif // PULSE_SOFTWARE_COMMAND_LIST_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
