// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include <stdio.h>

#include "../../PulseInternal.h"
#include "D3D11.h"
#include "D3D11Device.h"

bool PuD3D11IsDedicatedAdapter(IDXGIAdapter1* adapter)
{
	DXGI_ADAPTER_DESC1 desc;
	if(FAILED(adapter->lpVtbl->GetDesc1(adapter, &desc)))
		return false;
	bool has_dedicated_memory = desc.DedicatedVideoMemory > 0;
	bool is_software_adapter = wcsstr(desc.Description, L"Microsoft Basic Render") != PULSE_NULLPTR;

	// Known vendor IDs
	const uint32_t NVIDIA = 0x10DE;
	const uint32_t AMD    = 0x1002;

	bool is_known_vendor = (desc.VendorId == NVIDIA || desc.VendorId == AMD);

	return has_dedicated_memory && is_known_vendor && !is_software_adapter;
}

static uint64_t PuD3D11ScoreAdapter(ID3D11Device* device, IDXGIAdapter1* adapter)
{
}

PulseDevice PuD3D11CreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULLPTR);

	PulseDevice pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);

	D3D11Device* device = (D3D11Device*)calloc(1, sizeof(D3D11Device));
	PULSE_CHECK_ALLOCATION_RETVAL(device, PULSE_NULL_HANDLE);

	pulse_device->driver_data = device;
	pulse_device->backend = backend;

	CreateDXGIFactory1(&IID_IDXGIFactory, (void**)&device->factory);
	for(uint32_t i = 0; device->factory->lpVtbl->EnumAdapters1(device->factory, i, &device->adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		device->adapter->lpVtbl->GetDesc1(device->adapter, &desc);
		device->adapter->lpVtbl->Release(device->adapter);
	}

	device->factory->lpVtbl->Release(device->factory);
	return pulse_device;
}

void PuD3D11DestroyDevice(PulseDevice device)
{
}
