// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_BUFFER_H_
#define PULSE_SOFTWARE_BUFFER_H_

#include "Soft.h"

typedef struct SoftBuffer
{
	void* buffer;
	void* map;
	PulseMapMode current_map_mode;
} SoftBuffer;

PulseBuffer SoftCreateBuffer(PulseDevice device, const PulseBufferCreateInfo* create_infos);
bool SoftMapBuffer(PulseBuffer buffer, PulseMapMode mode, void** data);
void SoftUnmapBuffer(PulseBuffer buffer);
bool SoftCopyBufferToBuffer(PulseCommandList cmd, const PulseBufferRegion* src, const PulseBufferRegion* dst);
bool SoftCopyBufferToImage(PulseCommandList cmd, const PulseBufferRegion* src, const PulseImageRegion* dst);
void SoftDestroyBuffer(PulseDevice device, PulseBuffer buffer);

#endif // PULSE_SOFTWARE_BUFFER_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
