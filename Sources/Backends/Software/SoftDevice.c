// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <cpuinfo.h>

#include <Pulse.h>
#include "../../PulseInternal.h"
#include "Soft.h"
#include "SoftComputePipeline.h"
#include "SoftCommandList.h"
#include "SoftDevice.h"
#include "SoftFence.h"
#include "SoftBuffer.h"
#include "SoftImage.h"
#include "SoftComputePass.h"

PulseDevice SoftCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_UNUSED(forbiden_devices);
	PULSE_UNUSED(forbiden_devices_count);

	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULLPTR);

	PulseDevice pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);

	SoftDevice* device = (SoftDevice*)calloc(1, sizeof(SoftDevice));
	PULSE_CHECK_ALLOCATION_RETVAL(device, PULSE_NULL_HANDLE);

	device->device = cpuinfo_get_current_processor();
	device->spv_context = spvm_context_initialize();

	pulse_device->driver_data = device;
	pulse_device->backend = backend;
	PULSE_LOAD_DRIVER_DEVICE(Soft);

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
		PulseLogInfoFmt(backend, "(Soft) created device from %s", device->device->package->name);
	return pulse_device;
}

void SoftDestroyDevice(PulseDevice device)
{
	SoftDevice* soft_device = SOFT_RETRIEVE_DRIVER_DATA_AS(device, SoftDevice*);
	if(soft_device == PULSE_NULLPTR)
		return;
	spvm_context_deinitialize(soft_device->spv_context);
	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(Soft) destroyed device created from %s", soft_device->device->package->name);
	free(soft_device);
	free(device);
}
