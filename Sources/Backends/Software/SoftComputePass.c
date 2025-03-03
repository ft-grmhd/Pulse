// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftComputePass.h"

PulseComputePass SoftCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
	PULSE_UNUSED(device);
	PulseComputePass pass = (PulseComputePass)calloc(1, sizeof(PulseComputePassHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pass, PULSE_NULL_HANDLE);

	SoftComputePass* soft_pass = (SoftComputePass*)calloc(1, sizeof(SoftComputePass));
	PULSE_CHECK_ALLOCATION_RETVAL(soft_pass, PULSE_NULL_HANDLE);

	pass->cmd = cmd;
	pass->driver_data = soft_pass;

	return pass;
}

void SoftDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
	PULSE_UNUSED(device);
	free(pass->driver_data);
	free(pass);
}

PulseComputePass SoftBeginComputePass(PulseCommandList cmd)
{
}

void SoftEndComputePass(PulseComputePass pass)
{
}

void SoftBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
}

void SoftBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void SoftBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
}

void SoftBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
}

void SoftDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
}
