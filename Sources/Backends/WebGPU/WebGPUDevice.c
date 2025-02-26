// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "WebGPU.h"
#include "WebGPUComputePipeline.h"
#include "WebGPUCommandList.h"
#include "WebGPUDevice.h"
#include "WebGPUFence.h"
#include "WebGPUBuffer.h"
#include "WebGPUImage.h"
#include "WebGPUComputePass.h"
#include "webgpu.h"

#ifndef PULSE_PLAT_WASM
	#include <wgpu.h>
#endif

#ifdef PULSE_PLAT_WASM
	static void WebGPURequestAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2)
	{
		WebGPUDevice* device = (WebGPUDevice*)userdata1;
		PulseBackend backend = (PulseBackend)userdata2;
		if(status != WGPURequestAdapterStatus_Success)
		{
			if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
				PulseLogErrorFmt(backend, "(WebGPU) could not load adapter, %.*s", message.length, message.data);
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			device->has_error = true;
			return;
		}
		device->has_error = false;
		device->adapter = adapter;
	}
#else
	static uint64_t WebGPUScoreAdapter(WGPUInstance instance, WGPUAdapter adapter)
	{
		uint64_t score = 0;
		WGPUAdapterInfo infos;
		wgpuAdapterGetInfo(adapter, &infos);
		WGPULimits limits;
		wgpuAdapterGetLimits(adapter, &limits);

		if(infos.adapterType == WGPUAdapterType_DiscreteGPU)
			score += 10000;

		switch(infos.backendType)
		{
			case WGPUBackendType_Metal:
			case WGPUBackendType_D3D12:
			case WGPUBackendType_WebGPU:
			case WGPUBackendType_Vulkan: score += 1000;

			default: break;
		}

		score += limits.maxComputeWorkgroupSizeX;
		score += limits.maxComputeWorkgroupSizeY;
		score += limits.maxComputeWorkgroupSizeZ;
		score += limits.maxComputeWorkgroupStorageSize;
		score += limits.maxComputeInvocationsPerWorkgroup;
		score += limits.maxTextureDimension2D;

		return score;
	}
#endif

static bool WebGPUIsDeviceForbidden(WGPUAdapter adapter, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	if(adapter == PULSE_NULLPTR)
		return true;
	for(uint32_t i = 0; i < forbiden_devices_count; i++)
	{
		if(adapter == ((WebGPUDevice*)forbiden_devices[i]->driver_data)->adapter)
			return true;
	}
	return false;
}

static void WebGPURequestDeviceCallback(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* userdata2)
{
	WebGPUDevice* pulse_device = (WebGPUDevice*)userdata1;
	PulseBackend backend = (PulseBackend)userdata2;
	if(status != WGPURequestDeviceStatus_Success)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
			PulseLogErrorFmt(backend, "(WebGPU) could not create device from %.*s, %.*s", pulse_device->infos.device.length, pulse_device->infos.device.data, message.length, message.data);
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		pulse_device->has_error = true;
		return;
	}
	pulse_device->has_error = false;
	pulse_device->device = device;
}

static void WebGPUDeviceLostCallback(const WGPUDevice* _, WGPUDeviceLostReason reason, WGPUStringView message, void* userdata1, void* userdata2)
{
	WebGPUDevice* device = (WebGPUDevice*)userdata1;
	PulseBackend backend = (PulseBackend)userdata2;
	const char* reasons[] = {
		"of unknown reason",
		"device has been destroyed",
		"instance have been dropped",
		"creation failed",
	};
	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
		PulseLogErrorFmt(backend, "(WebGPU) device %.*s lost because %s. %.*s", device->infos.device.length, device->infos.device.data, reasons[reason], message.length, message.data);
}

static void WebGPUDeviceUncapturedErrorCallback(const WGPUDevice* _, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2)
{
	WebGPUDevice* device = (WebGPUDevice*)userdata1;
	PulseBackend backend = (PulseBackend)userdata2;
	const char* types[] = {
		"has recieved no error",
		"has recieved a validation error",
		"is out of memory",
		"has recieved an internal error",
		"has recieved an unknown error",
	};
	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(backend))
		PulseLogErrorFmt(backend, "(WebGPU) device %.*s %s. %.*s", device->infos.device.length, device->infos.device.data, types[type], message.length, message.data);
}

PulseDevice WebGPUCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULLPTR);

	PulseDevice pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);

	WebGPUDevice* device = (WebGPUDevice*)calloc(1, sizeof(WebGPUDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(device, PULSE_NULL_HANDLE);

	WGPUInstance instance = WEBGPU_RETRIEVE_DRIVER_DATA_AS(backend, WebGPUDriverData*)->instance;

	#ifdef PULSE_PLAT_WASM
		WGPURequestAdapterOptions adapter_options = { 0 };
		adapter_options.powerPreference = WGPUPowerPreference_HighPerformance;
		adapter_options.compatibleSurface = PULSE_NULLPTR;
		adapter_options.backendType = WGPUBackendType_Undefined;
		WGPURequestAdapterCallbackInfo adapter_callback = { 0 };
		adapter_callback.callback = WebGPURequestAdapterCallback;
		adapter_callback.mode = WGPUCallbackMode_AllowProcessEvents;
		adapter_callback.userdata1 = device;
		adapter_callback.userdata2 = backend;
		wgpuInstanceRequestAdapter(instance, &adapter_options, adapter_callback);

		while(device->adapter == PULSE_NULLPTR && !device->has_error) // Wait for adapter request
			PulseSleep(100);
		
		if(device->adapter != PULSE_NULLPTR && WebGPUIsDeviceForbidden(device->adapter, forbiden_devices, forbiden_devices_count))
		{
			if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
				PulseLogError(backend, "(WebGPU) could not find suitable adapter");
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			free(pulse_device);
			free(device);
			return PULSE_NULL_HANDLE;
		}
	#else
		WGPUInstanceEnumerateAdapterOptions adapter_options = { 0 };
		adapter_options.nextInChain = PULSE_NULLPTR;
		adapter_options.backends = WGPUInstanceBackend_All;
		size_t adapter_count = wgpuInstanceEnumerateAdapters(instance, &adapter_options, PULSE_NULLPTR);
		WGPUAdapter* adapters = (WGPUAdapter*)calloc(adapter_count, sizeof(WGPUAdapter));
		wgpuInstanceEnumerateAdapters(instance, &adapter_options, adapters);
		uint64_t best_device_score = 0;
		for(uint32_t i = 0; i < adapter_count; i++)
		{
			if(WebGPUIsDeviceForbidden(adapters[i], forbiden_devices, forbiden_devices_count))
				continue;
			uint64_t current_device_score = WebGPUScoreAdapter(instance, adapters[i]);
			if(current_device_score > best_device_score)
			{
				best_device_score = current_device_score;
				device->adapter = adapters[i];
			}
		}
	#endif

	if(device->adapter == PULSE_NULLPTR)
	{
		if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
			PulseLogError(backend, "(WebGPU) could not find suitable adapter");
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		free(pulse_device);
		free(device);
		return PULSE_NULL_HANDLE;
	}

	wgpuAdapterGetLimits(device->adapter, &device->limits);
	wgpuAdapterGetInfo(device->adapter, &device->infos);

	WGPUDeviceLostCallbackInfo lost_callback = { 0 };
	lost_callback.callback = WebGPUDeviceLostCallback;
	lost_callback.mode = WGPUCallbackMode_AllowProcessEvents;
	lost_callback.userdata1 = device;
	lost_callback.userdata2 = backend;
	WGPUUncapturedErrorCallbackInfo uncaptured_callback = { 0 };
	uncaptured_callback.callback = WebGPUDeviceUncapturedErrorCallback;
	uncaptured_callback.userdata1 = device;
	uncaptured_callback.userdata2 = backend;
	WGPUDeviceDescriptor descriptor = { 0 };
	descriptor.requiredLimits = &device->limits;
	descriptor.deviceLostCallbackInfo = lost_callback;
	descriptor.uncapturedErrorCallbackInfo = uncaptured_callback;
	WGPURequestDeviceCallbackInfo device_callback = { 0 };
	device_callback.callback = WebGPURequestDeviceCallback;
	device_callback.mode = WGPUCallbackMode_AllowProcessEvents;
	device_callback.userdata1 = device;
	device_callback.userdata2 = backend;
	wgpuAdapterRequestDevice(device->adapter, &descriptor, device_callback);

	while(device->device == PULSE_NULLPTR && !device->has_error) // Wait for device request
		PulseSleep(100);

	if(device->has_error)
	{
		free(pulse_device);
		free(device);
		return PULSE_NULL_HANDLE;
	}

	device->queue = wgpuDeviceGetQueue(device->device);

	pulse_device->driver_data = device;
	pulse_device->backend = backend;
	PULSE_LOAD_DRIVER_DEVICE(WebGPU);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
	{
		const char* backends[] = {
			"Undefined",
			"Null",
			"WebGPU",
			"D3D11",
			"D3D12",
			"Metal",
			"Vulkan",
			"OpenGL",
			"OpenGLES",
		};
		PulseLogInfoFmt(backend, "(WebGPU) created device from %.*s using %s backend", device->infos.device.length, device->infos.device.data, backends[device->infos.backendType]);
	}
	return pulse_device;
}

void WebGPUDestroyDevice(PulseDevice device)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);
	if(webgpu_device == PULSE_NULLPTR || webgpu_device->device == PULSE_NULLPTR)
		return;
	wgpuQueueRelease(webgpu_device->queue);
	wgpuDeviceRelease(webgpu_device->device);
	wgpuAdapterRelease(webgpu_device->adapter);
	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(WebGPU) destroyed device created from %.*s", webgpu_device->infos.device.length, webgpu_device->infos.device.data);
	wgpuAdapterInfoFreeMembers(webgpu_device->infos);
	free(webgpu_device);
	free(device);
}
