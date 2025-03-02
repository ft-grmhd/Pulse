// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_IMAGE_H_
#define PULSE_SOFTWARE_IMAGE_H_

#include "Soft.h"

typedef struct SoftImage
{
} SoftImage;

PulseImage SoftCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos);
bool SoftIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage);
bool SoftCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst);
bool SoftBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst);
void SoftDestroyImage(PulseDevice device, PulseImage image);

#endif // PULSE_SOFTWARE_IMAGE_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
