// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdlib.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "Vulkan.h"
#include "VulkanLoader.h"
#include "VulkanInstance.h"

VulkanGlobal* VulkanGetGlobal()
{
	static VulkanGlobal vulkan = { 0 };
	return &vulkan;
}

PulseBackendFlags VulkanCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_VULKAN) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_SPIRV_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	if(!VulkanInitLoader())
		return PULSE_BACKEND_INVALID;
	VulkanInstance instance;
	if(!VulkanInitInstance(&instance, PULSE_NO_DEBUG)) // Instance creation will fail if Vulkan is not supported
	{
		PulseGetLastErrorType(); // Clearing out the errors set by the failed instance creation
		return PULSE_BACKEND_INVALID;
	}
	VulkanDestroyInstance(&instance);
	VulkanLoaderShutdown();
	return PULSE_BACKEND_VULKAN;
}

bool VulkanLoadBackend(PulseDebugLevel debug_level)
{
	if(!VulkanInitLoader())
		return false;
	VulkanDriverData* driver_data = (VulkanDriverData*)calloc(1, sizeof(VulkanDriverData));
	PULSE_CHECK_ALLOCATION_RETVAL(driver_data, false);
	if(!VulkanInitInstance(&driver_data->instance, debug_level))
		return false;
	VulkanDriver.driver_data = driver_data;
	return true;
}

void VulkanUnloadBackend(PulseBackend backend)
{
	VulkanDestroyInstance(&VULKAN_RETRIEVE_DRIVER_DATA(backend)->instance);
	VulkanLoaderShutdown();
}

PulseBackendHandler VulkanDriver = {
	.PFN_LoadBackend = VulkanLoadBackend,
	.PFN_UnloadBackend = VulkanUnloadBackend,
	.PFN_CreateDevice = VulkanCreateDevice,
	.backend = PULSE_BACKEND_VULKAN,
	.supported_shader_formats = PULSE_SHADER_FORMAT_SPIRV_BIT,
	.driver_data = PULSE_NULLPTR
};
