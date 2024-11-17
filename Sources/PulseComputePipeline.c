// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "PulseInternal.h"

PULSE_API PulseComputePipeline PulseCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
}

PULSE_API void PulseBindComputePipeline(PulseComputePipeline pipeline)
{
	PULSE_CHECK_HANDLE(pipeline);
}

PULSE_API void PulseDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	PULSE_CHECK_HANDLE(device);
}
