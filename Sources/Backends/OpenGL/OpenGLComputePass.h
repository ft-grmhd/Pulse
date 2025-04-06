// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_COMPUTE_PASS_H_
#define PULSE_OPENGL_COMPUTE_PASS_H_

#include <Pulse.h>
#include "OpenGL.h"
#include "OpenGLBuffer.h"
#include "OpenGLCommandList.h"
#include "OpenGLBindsGroup.h"

typedef struct OpenGLComputePass
{
	OpenGLBindsGroup* read_only_bind_group;
	OpenGLBindsGroup* read_write_bind_group;
	OpenGLBindsGroup* uniform_bind_group;

	bool should_recreate_read_only_bind_group;
	bool should_recreate_write_bind_group;
	bool should_recreate_uniform_bind_group;
} OpenGLComputePass;

PulseComputePass OpenGLCreateComputePass(PulseDevice device, PulseCommandList cmd);
void OpenGLDestroyComputePass(PulseDevice device, PulseComputePass pass);

PulseComputePass OpenGLBeginComputePass(PulseCommandList cmd);
void OpenGLEndComputePass(PulseComputePass pass);
void OpenGLBindStorageBuffers(PulseComputePass pass, const PulseBuffer* buffers, uint32_t num_buffers);
void OpenGLBindUniformData(PulseComputePass pass, uint32_t slot, const void* data, uint32_t data_size);
void OpenGLBindStorageImages(PulseComputePass pass, const PulseImage* images, uint32_t num_images);
void OpenGLBindComputePipeline(PulseComputePass pass, PulseComputePipeline pipeline);
void OpenGLDispatchComputations(PulseComputePass pass, uint32_t groupcount_x, uint32_t groupcount_y, uint32_t groupcount_z);

#endif // PULSE_OPENGL_COMPUTE_PASS_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND

