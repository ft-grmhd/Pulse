// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLComputePass.h"

PulseComputePass OpenGLCreateComputePass(PulseDevice device, PulseCommandList cmd)
{
}

void OpenGLDestroyComputePass(PulseDevice device, PulseComputePass pass)
{
}

PulseComputePass OpenGLBeginComputePass(PulseCommandList cmd)
{
}

void OpenGLEndComputePass(PulseComputePass pass)
{
}

void OpenGLBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers)
{
}

void OpenGLBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size)
{
}

void OpenGLBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images)
{
}

void OpenGLBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline)
{
}

void OpenGLDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z)
{
}
