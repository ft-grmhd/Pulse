// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftBuffer.h"

PulseBuffer SoftCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos)
{
}

bool SoftMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data)
{
}

void SoftUnmapBuffer(PulseBuffer buffer)
{
}

bool SoftCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst)
{
}

bool SoftCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst)
{
}

void SoftDestroyBuffer(PulseDevice device, PulseBuffer buffer)
{
}
