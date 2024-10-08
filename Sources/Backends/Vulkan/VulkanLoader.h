// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_VULKAN_BACKEND

#ifndef PULSE_VULKAN_LOADER_H_
#define PULSE_VULKAN_LOADER_H_

#include <stdbool.h>

#include "VulkanDevice.h"
#include "VulkanInstance.h"

bool VulkanInitLoader();
bool VulkanLoadInstance(VulkanInstance* instance);
bool VulkanLoadDevice(VulkanInstance* instance, VulkanDevice* device);
void VulkanLoaderShutdown();

#endif // PULSE_VULKAN_LOADER_H_

#endif // PULSE_ENABLE_VULKAN_BACKEND
