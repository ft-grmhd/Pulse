// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_FENCE_H_
#define PULSE_VULKAN_FENCE_H_

#include <Pulse.h>
#include "VulkanDevice.h"

PulseFence VulkanCreateFence(PulseDevice device);
void VulkanDestroyFence(PulseDevice device, PulseFence fence);
bool VulkanIsFenceReady(PulseDevice device, PulseFence fence);
bool VulkanWaitForFences(PulseDevice device, const PulseFence* fences, uint32_t fences_count, bool wait_for_all);

#endif // PULSE_VULKAN_FENCE_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
