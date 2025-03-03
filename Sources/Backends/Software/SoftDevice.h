// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_DEVICE_H_
#define PULSE_SOFTWARE_DEVICE_H_

#include "Soft.h"

typedef struct SoftDevice
{
	PulseCommandList* available_command_lists;
	uint32_t available_command_lists_capacity;
	uint32_t available_command_lists_size;
} SoftDevice;

PulseDevice SoftCreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count);
void SoftDestroyDevice(PulseDevice device);

#endif // PULSE_SOFTWARE_DEVICE_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
