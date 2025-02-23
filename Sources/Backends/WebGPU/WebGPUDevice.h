// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_WEBGPU_BACKEND

#ifndef PULSE_WEBGPU_DEVICE_H_
#define PULSE_WEBGPU_DEVICE_H_

#include <webgpu/webgpu.h>

#include <Pulse.h>

typedef struct WebGPUDevice
{
	WGPUAdapterInfo infos;
	WGPULimits limits;
	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUQueue queue;

	bool has_error;
} WebGPUDevice;

PulseDevice WebGPUCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count);
void WebGPUDestroyDevice(PulseDevice device);

#endif // PULSE_WEBGPU_DEVICE_H_

#endif // PULSE_ENABLE_WEBGPU_BACKEND
