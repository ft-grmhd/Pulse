// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifdef PULSE_ENABLE_D3D11_BACKEND

#ifndef PULSE_D3D11_DEVICE_H_
#define PULSE_D3D11_DEVICE_H_

#include <Pulse.h>
#include "D3D11.h"

typedef struct Direct3D11Device
{
	DXGI_ADAPTER_DESC1 description;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGIFactory1* factory;
	IDXGIAdapter1* adapter;
} Direct3D11Device;

PulseDevice Direct3D11CreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count);
void Direct3D11DestroyDevice(PulseDevice device);

#endif // PULSE_D3D11_DEVICE_H_

#endif // PULSE_ENABLE_D3D11_BACKEND
