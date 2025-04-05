// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_COMPUTE_PASS_H_
#define PULSE_SOFTWARE_COMPUTE_PASS_H_

#include "Soft.h"

PulseComputePass SoftCreateComputePass(PulseDevice device, PulseCommandList cmd);
void SoftDestroyComputePass(PulseDevice device, PulseComputePass pass);

PulseComputePass SoftBeginComputePass(PulseCommandList cmd);
void SoftEndComputePass(PulseComputePass pass);
void SoftBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers);
void SoftBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size);
void SoftBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images);
void SoftBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline);
void SoftDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);

#endif // PULSE_SOFTWARE_COMPUTE_PASS_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
