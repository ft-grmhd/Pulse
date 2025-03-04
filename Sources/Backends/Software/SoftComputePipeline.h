// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_COMPUTE_PIPELINE_H_
#define PULSE_SOFTWARE_COMPUTE_PIPELINE_H_

#include "Soft.h"
#include <spvm/state.h>
#include <spvm/program.h>

typedef struct SoftComputePipeline
{
	spvm_program_t program;
	const char* entry_point;
} SoftComputePipeline;

PulseComputePipeline SoftCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info);
void SoftDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline);

#endif // PULSE_SOFTWARE_COMPUTE_PIPELINE_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
