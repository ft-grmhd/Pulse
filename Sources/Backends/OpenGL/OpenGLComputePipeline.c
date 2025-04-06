// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLDevice.h"
#include "OpenGLBindsGroup.h"
#include "OpenGLComputePipeline.h"

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
	GLint linked = GL_FALSE;
	opengl_device->glGetProgramiv(device, opengl_pipeline->program, GL_LINK_STATUS, &linked);
	if(linked != GL_TRUE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
		{
			GLint len = 0;
			opengl_device->glGetProgramiv(device, opengl_pipeline->program, GL_INFO_LOG_LENGTH, &len);

			char* log = (char*)malloc(len);
			PULSE_CHECK_ALLOCATION_RETVAL(log, PULSE_NULL_HANDLE);
			opengl_device->glGetProgramInfoLog(device, opengl_pipeline->program, len, PULSE_NULLPTR, log);

			PulseLogErrorFmt(device->backend, "compute pipeline could not be created\n%.*s", len, log);

			free(log);
		}

		opengl_device->glDeleteProgram(device, opengl_pipeline->program);
		free(opengl_pipeline);
		free(pipeline);
		return PULSE_NULL_HANDLE;
	}

	opengl_pipeline->readonly_group_layout  = OpenGLGetBindsGroupLayout(&opengl_device->binds_group_layout_manager, info->num_readonly_storage_images, info->num_readonly_storage_buffers, 0, 0, 0);
	opengl_pipeline->readwrite_group_layout = OpenGLGetBindsGroupLayout(&opengl_device->binds_group_layout_manager, 0, 0, info->num_readwrite_storage_images, info->num_readwrite_storage_buffers, 0);
	opengl_pipeline->uniform_group_layout   = OpenGLGetBindsGroupLayout(&opengl_device->binds_group_layout_manager, 0, 0, 0, 0, info->num_uniform_buffers);

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

	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
	OpenGLComputePipeline* opengl_pipeline = OPENGL_RETRIEVE_DRIVER_DATA_AS(pipeline, OpenGLComputePipeline*);
	opengl_device->glDeleteProgram(device, opengl_pipeline->program);
	free(opengl_pipeline);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "%s destroyed compute pipeline %p", device->backend->backend == PULSE_BACKEND_OPENGL ? "(OpenGL)" : "(OpenGL ES)", pipeline);

	free(pipeline);
}
