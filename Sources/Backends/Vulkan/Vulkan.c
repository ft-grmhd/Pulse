// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdlib.h>

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

PulseDevice VulkanCreatePulseDevice(PulseDebugLevel debug_level)
{
	VulkanPulseDevice* vulkan_device = (VulkanPulseDevice*)calloc(1, sizeof(VulkanPulseDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(vulkan_device, PULSE_NULL_HANDLE);

	if(!VulkanInitInstance(&vulkan_device->instance, debug_level))
		return PULSE_NULL_HANDLE;

	PulseDeviceHandler* pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);
	pulse_device->debug_level = debug_level;
	pulse_device->driver_data = vulkan_device;
	return pulse_device;
}

void VulkanDestroyPulseDevice(PulseDevice device)
{
	
}

PulseBackendLoader VulkanDriver = {
	.PFN_LoadBackend = VulkanLoadBackend,
	.PFN_CreateDevice = VulkanCreatePulseDevice,
	.PFN_DestroyDevice = VulkanDestroyPulseDevice,
	.backend = PULSE_BACKEND_VULKAN,
	.supported_shader_formats = PULSE_SHADER_FORMAT_SPIRV_BIT
};
