// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGLBuffer.h"

PulseBuffer OpenGLCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
}

bool OpenGLMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data)
{
}

void OpenGLUnmapBuffer(PulseBuffer buffer)
{
}

bool OpenGLCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
}

bool OpenGLCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
}

void OpenGLDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
}
