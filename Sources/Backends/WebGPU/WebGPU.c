// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "WebGPU.h"
#include "WebGPUDevice.h"

void WebGPUDeviceTick(PulseDevice device)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);
	wgpuQueueSubmit(webgpu_device->queue, 0, PULSE_NULLPTR); // Submitting nothing just to check for ongoing asynchronous operations and call their callbacks if needed
}

PulseBackendFlags WebGPUCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used)
{
	if(candidates != PULSE_BACKEND_ANY && (candidates & PULSE_BACKEND_WEBGPU) == 0)
		return PULSE_BACKEND_INVALID;
	if((shader_formats_used & PULSE_SHADER_FORMAT_WGSL_BIT) == 0)
		return PULSE_BACKEND_INVALID;

	WGPUInstance instance = wgpuCreateInstance(PULSE_NULLPTR);
	if(instance == PULSE_NULLPTR)
		return PULSE_BACKEND_INVALID;
	wgpuInstanceRelease(instance);
	return PULSE_BACKEND_WEBGPU;
}

bool WebGPULoadBackend(PulseBackend backend, PulseDebugLevel debug_level)
{
	PULSE_UNUSED(backend);
	PULSE_UNUSED(debug_level);
	WebGPUDriverData* driver_data = (WebGPUDriverData*)calloc(1, sizeof(WebGPUDriverData));
	PULSE_CHECK_ALLOCATION_RETVAL(driver_data, false);
	driver_data->instance = wgpuCreateInstance(PULSE_NULLPTR);
	if(driver_data->instance == PULSE_NULLPTR)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		free(driver_data);
		return false;
	}
	WebGPUDriver.driver_data = driver_data;
	return true;
}

void WebGPUUnloadBackend(PulseBackend backend)
{
	wgpuInstanceRelease(WEBGPU_RETRIEVE_DRIVER_DATA_AS(backend, WebGPUDriverData*)->instance);
	free(backend->driver_data);
}

PulseBackendHandler WebGPUDriver = {
	.PFN_LoadBackend = WebGPULoadBackend,
	.PFN_UnloadBackend = WebGPUUnloadBackend,
	.PFN_CreateDevice = WebGPUCreateDevice,
	.backend = PULSE_BACKEND_WEBGPU,
	.supported_shader_formats = PULSE_SHADER_FORMAT_WGSL_BIT,
	.driver_data = PULSE_NULLPTR
};

int32_t WebGPUGetImageBlockWidth(PulseImageFormat format)
{
	switch(format)
	{
		case PULSE_IMAGE_FORMAT_BC1_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC2_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC3_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC4_R_UNORM:
		case PULSE_IMAGE_FORMAT_BC5_RG_UNORM:
		case PULSE_IMAGE_FORMAT_BC7_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC6H_RGB_FLOAT:
		case PULSE_IMAGE_FORMAT_BC6H_RGB_UFLOAT: return 4;

		case PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM:
		case PULSE_IMAGE_FORMAT_B8G8R8A8_UNORM:
		case PULSE_IMAGE_FORMAT_B5G6R5_UNORM:
		case PULSE_IMAGE_FORMAT_B5G5R5A1_UNORM:
		case PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM:
		case PULSE_IMAGE_FORMAT_R10G10B10A2_UNORM:
		case PULSE_IMAGE_FORMAT_R8G8_UNORM:
		case PULSE_IMAGE_FORMAT_R16G16_UNORM:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_UNORM:
		case PULSE_IMAGE_FORMAT_R8_UNORM:
		case PULSE_IMAGE_FORMAT_R16_UNORM:
		case PULSE_IMAGE_FORMAT_A8_UNORM:
		case PULSE_IMAGE_FORMAT_R8_SNORM:
		case PULSE_IMAGE_FORMAT_R8G8_SNORM:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_SNORM:
		case PULSE_IMAGE_FORMAT_R16_SNORM:
		case PULSE_IMAGE_FORMAT_R16G16_SNORM:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_SNORM:
		case PULSE_IMAGE_FORMAT_R16_FLOAT:
		case PULSE_IMAGE_FORMAT_R16G16_FLOAT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_FLOAT:
		case PULSE_IMAGE_FORMAT_R32_FLOAT:
		case PULSE_IMAGE_FORMAT_R32G32_FLOAT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_FLOAT:
		case PULSE_IMAGE_FORMAT_R11G11B10_UFLOAT:
		case PULSE_IMAGE_FORMAT_R8_UINT:
		case PULSE_IMAGE_FORMAT_R8G8_UINT:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_UINT:
		case PULSE_IMAGE_FORMAT_R16_UINT:
		case PULSE_IMAGE_FORMAT_R16G16_UINT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_UINT:
		case PULSE_IMAGE_FORMAT_R32_UINT:
		case PULSE_IMAGE_FORMAT_R32G32_UINT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_UINT:
		case PULSE_IMAGE_FORMAT_R8_INT:
		case PULSE_IMAGE_FORMAT_R8G8_INT:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_INT:
		case PULSE_IMAGE_FORMAT_R16_INT:
		case PULSE_IMAGE_FORMAT_R16G16_INT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_INT:
		case PULSE_IMAGE_FORMAT_R32_INT:
		case PULSE_IMAGE_FORMAT_R32G32_INT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_INT: return 1;

		default: return 0;
    }
}

int32_t WebGPUGetImageBlockHeight(PulseImageFormat format)
{
	switch(format)
	{
		case PULSE_IMAGE_FORMAT_BC1_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC2_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC3_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC4_R_UNORM:
		case PULSE_IMAGE_FORMAT_BC5_RG_UNORM:
		case PULSE_IMAGE_FORMAT_BC7_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC6H_RGB_FLOAT:
		case PULSE_IMAGE_FORMAT_BC6H_RGB_UFLOAT: return 4;

		case PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM:
		case PULSE_IMAGE_FORMAT_B8G8R8A8_UNORM:
		case PULSE_IMAGE_FORMAT_B5G6R5_UNORM:
		case PULSE_IMAGE_FORMAT_B5G5R5A1_UNORM:
		case PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM:
		case PULSE_IMAGE_FORMAT_R10G10B10A2_UNORM:
		case PULSE_IMAGE_FORMAT_R8G8_UNORM:
		case PULSE_IMAGE_FORMAT_R16G16_UNORM:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_UNORM:
		case PULSE_IMAGE_FORMAT_R8_UNORM:
		case PULSE_IMAGE_FORMAT_R16_UNORM:
		case PULSE_IMAGE_FORMAT_A8_UNORM:
		case PULSE_IMAGE_FORMAT_R8_SNORM:
		case PULSE_IMAGE_FORMAT_R8G8_SNORM:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_SNORM:
		case PULSE_IMAGE_FORMAT_R16_SNORM:
		case PULSE_IMAGE_FORMAT_R16G16_SNORM:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_SNORM:
		case PULSE_IMAGE_FORMAT_R16_FLOAT:
		case PULSE_IMAGE_FORMAT_R16G16_FLOAT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_FLOAT:
		case PULSE_IMAGE_FORMAT_R32_FLOAT:
		case PULSE_IMAGE_FORMAT_R32G32_FLOAT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_FLOAT:
		case PULSE_IMAGE_FORMAT_R11G11B10_UFLOAT:
		case PULSE_IMAGE_FORMAT_R8_UINT:
		case PULSE_IMAGE_FORMAT_R8G8_UINT:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_UINT:
		case PULSE_IMAGE_FORMAT_R16_UINT:
		case PULSE_IMAGE_FORMAT_R16G16_UINT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_UINT:
		case PULSE_IMAGE_FORMAT_R32_UINT:
		case PULSE_IMAGE_FORMAT_R32G32_UINT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_UINT:
		case PULSE_IMAGE_FORMAT_R8_INT:
		case PULSE_IMAGE_FORMAT_R8G8_INT:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_INT:
		case PULSE_IMAGE_FORMAT_R16_INT:
		case PULSE_IMAGE_FORMAT_R16G16_INT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_INT:
		case PULSE_IMAGE_FORMAT_R32_INT:
		case PULSE_IMAGE_FORMAT_R32G32_INT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_INT: return 1;

		default: return 0;
	}
}

uint32_t WebGPUImageFormatTexelBlockSize(PulseImageFormat format)
{
	switch(format)
	{
		case PULSE_IMAGE_FORMAT_BC1_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC4_R_UNORM: return 8;

		case PULSE_IMAGE_FORMAT_BC2_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC3_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC5_RG_UNORM:
		case PULSE_IMAGE_FORMAT_BC7_RGBA_UNORM:
		case PULSE_IMAGE_FORMAT_BC6H_RGB_FLOAT:
		case PULSE_IMAGE_FORMAT_BC6H_RGB_UFLOAT: return 16;

		case PULSE_IMAGE_FORMAT_R8_UNORM:
		case PULSE_IMAGE_FORMAT_R8_SNORM:
		case PULSE_IMAGE_FORMAT_A8_UNORM:
		case PULSE_IMAGE_FORMAT_R8_UINT:
		case PULSE_IMAGE_FORMAT_R8_INT: return 1;

		case PULSE_IMAGE_FORMAT_B5G6R5_UNORM:
		case PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM:
		case PULSE_IMAGE_FORMAT_B5G5R5A1_UNORM:
		case PULSE_IMAGE_FORMAT_R16_FLOAT:
		case PULSE_IMAGE_FORMAT_R8G8_SNORM:
		case PULSE_IMAGE_FORMAT_R8G8_UNORM:
		case PULSE_IMAGE_FORMAT_R8G8_UINT:
		case PULSE_IMAGE_FORMAT_R8G8_INT:
		case PULSE_IMAGE_FORMAT_R16_UNORM:
		case PULSE_IMAGE_FORMAT_R16_SNORM:
		case PULSE_IMAGE_FORMAT_R16_UINT:
		case PULSE_IMAGE_FORMAT_R16_INT: return 2;

		case PULSE_IMAGE_FORMAT_R8G8B8A8_UNORM:
		case PULSE_IMAGE_FORMAT_B8G8R8A8_UNORM:
		case PULSE_IMAGE_FORMAT_R32_FLOAT:
		case PULSE_IMAGE_FORMAT_R16G16_FLOAT:
		case PULSE_IMAGE_FORMAT_R11G11B10_UFLOAT:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_SNORM:
		case PULSE_IMAGE_FORMAT_R10G10B10A2_UNORM:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_UINT:
		case PULSE_IMAGE_FORMAT_R8G8B8A8_INT:
		case PULSE_IMAGE_FORMAT_R16G16_UINT:
		case PULSE_IMAGE_FORMAT_R16G16_INT:
		case PULSE_IMAGE_FORMAT_R16G16_UNORM:
		case PULSE_IMAGE_FORMAT_R16G16_SNORM:
		case PULSE_IMAGE_FORMAT_R32_UINT:
		case PULSE_IMAGE_FORMAT_R32_INT: return 4;

		case PULSE_IMAGE_FORMAT_R16G16B16A16_FLOAT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_UNORM:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_SNORM:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_UINT:
		case PULSE_IMAGE_FORMAT_R16G16B16A16_INT:
		case PULSE_IMAGE_FORMAT_R32G32_FLOAT:
		case PULSE_IMAGE_FORMAT_R32G32_UINT:
		case PULSE_IMAGE_FORMAT_R32G32_INT: return 8;

		case PULSE_IMAGE_FORMAT_R32G32B32A32_FLOAT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_INT:
		case PULSE_IMAGE_FORMAT_R32G32B32A32_UINT: return 16;

		default: return 0;
    }
}

uint32_t WebGPUBytesPerRow(int32_t width, PulseImageFormat format)
{
	uint32_t block_width = WebGPUGetImageBlockWidth(format);
	if(block_width == 0)
		return 0;
	uint32_t blocks_per_row = (width + block_width - 1) / block_width;
	return blocks_per_row * WebGPUImageFormatTexelBlockSize(format);
}
