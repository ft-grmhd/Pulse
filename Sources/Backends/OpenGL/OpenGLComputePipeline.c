// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLComputePipeline.h"
#include "OpenGLDevice.h"

PulseComputePipeline OpenGLCreateComputePipeline(PulseDevice device, const PulseComputePipelineCreateInfo* info)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	PulseComputePipelineHandler* pipeline = (PulseComputePipelineHandler*)calloc(1, sizeof(PulseComputePipelineHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pipeline, PULSE_NULL_HANDLE);

	OpenGLComputePipeline* opengl_pipeline = (OpenGLComputePipeline*)calloc(1, sizeof(OpenGLComputePipeline));
	PULSE_CHECK_ALLOCATION_RETVAL(opengl_pipeline, PULSE_NULL_HANDLE);

	pipeline->driver_data = opengl_pipeline;

	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(info->code == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid code pointer passed to PulseComputePipelineCreateInfo");
		if(info->entrypoint == PULSE_NULLPTR)
			PulseLogError(device->backend, "invalid entrypoint pointer passed to PulseComputePipelineCreateInfo");
		if(info->format == PULSE_SHADER_FORMAT_WGSL_BIT && (device->backend->supported_shader_formats & PULSE_SHADER_FORMAT_WGSL_BIT) == 0)
			PulseLogError(device->backend, "invalid shader format passed to PulseComputePipelineCreateInfo");
	}

	opengl_pipeline->program = opengl_device->glCreateShaderProgramv(device, GL_COMPUTE_SHADER, 1, (const GLchar**)(&info->code));

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "%s created new compute pipeline %p", device->backend->backend == PULSE_BACKEND_OPENGL ? "(OpenGL)" : "(OpenGL ES)", pipeline);
	return pipeline;
}

void OpenGLDestroyComputePipeline(PulseDevice device, PulseComputePipeline pipeline)
{
	if(pipeline == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "compute pipeline is NULL, this may be a bug in your application");
		return;
	}

	PULSE_UNUSED(device);
	OpenGLComputePipeline* opengl_pipeline = OPENGL_RETRIEVE_DRIVER_DATA_AS(pipeline, OpenGLComputePipeline*);
	free(opengl_pipeline);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "%s destroyed compute pipeline %p", device->backend->backend == PULSE_BACKEND_OPENGL ? "(OpenGL)" : "(OpenGL ES)", pipeline);

	free(pipeline);
}
