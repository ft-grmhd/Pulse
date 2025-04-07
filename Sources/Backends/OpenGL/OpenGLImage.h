// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_IMAGE_H_
#define PULSE_OPENGL_IMAGE_H_

#include <Pulse.h>
#include "OpenGL.h"

typedef struct OpenGLImage
{
	GLuint image;
} OpenGLImage;

PulseImage OpenGLCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos);
bool OpenGLIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage);
bool OpenGLCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst);
bool OpenGLBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst);
void OpenGLDestroyImage(PulseDevice device, PulseImage image);

#endif // PULSE_OPENGL_IMAGE_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
