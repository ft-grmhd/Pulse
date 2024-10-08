// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "Vulkan.h"
#include "VulkanLoader.h"

VulkanGlobal* VulkanGetGlobal()
{
	static VulkanGlobal vulkan = { 0 };
	return &vulkan;
}

bool VulkanLoadBackend()
{
	if(!VulkanInitLoader())
		return false;

	return true;
}

PulseBackendLoader VulkanDriver = {
	.PFN_LoadBackend = VulkanLoadBackend,
	.PFN_CreateDevice = PULSE_NULLPTR,
	.backend = PULSE_BACKEND_VULKAN,
	.supported_shader_formats = PULSE_SHADER_FORMAT_SPIRV_BIT
};
