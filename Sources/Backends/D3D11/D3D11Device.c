// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include <wchar.h>

#include "../../PulseInternal.h"
#include "D3D11.h"
#include "D3D11Device.h"

bool Direct3D11IsDedicatedAdapter(IDXGIAdapter1* adapter)
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

static uint64_t Direct3D11ScoreAdapter(IDXGIAdapter1* adapter)
{
	DXGI_ADAPTER_DESC1 desc;
	adapter->lpVtbl->GetDesc1(adapter, &desc);

	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1, // :doubt:
		D3D_FEATURE_LEVEL_10_0
	};

	ID3D11Device* base_device = PULSE_NULLPTR;
	ID3D11DeviceContext* base_context = PULSE_NULLPTR;
	ID3D11Device1* d3d_device1 = PULSE_NULLPTR;
	ID3D11DeviceContext1* d3d_context1 = PULSE_NULLPTR;

	D3D_FEATURE_LEVEL out_feature_level;

	uint64_t score = 0;

	HRESULT hr = D3D11CreateDevice((IDXGIAdapter*)adapter, D3D_DRIVER_TYPE_UNKNOWN, PULSE_NULLPTR, 0, feature_levels, PULSE_SIZEOF_ARRAY(feature_levels), D3D11_SDK_VERSION, &base_device, &out_feature_level, &base_context);

	if(FAILED(hr))
		return 0;

	hr = base_device->lpVtbl->QueryInterface(base_device, &IID_ID3D11Device1, (void**)&d3d_device1);
	if(FAILED(hr) || out_feature_level < D3D_FEATURE_LEVEL_10_0)
		goto Cleanup;

	if(Direct3D11IsDedicatedAdapter(adapter))
		score += 50000;
	else if(!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
	{
		// Often integrated GPUs are labeled "Intel(R)" in the description
		if(wcsstr(desc.Description, L"Intel") != PULSE_NULLPTR)
			score += 10000;
	}

	score += desc.DedicatedVideoMemory;
	score += desc.DedicatedSystemMemory;
	score += desc.SharedSystemMemory;

Cleanup:
	if(base_device)
		base_device->lpVtbl->Release(base_device);
	if(base_context)
		base_context->lpVtbl->Release(base_context);
	return score;
}

static bool Direct3D11IsAdapterForbidden(IDXGIAdapter1* adapter, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	if(adapter == PULSE_NULLPTR)
		return true;
	for(uint32_t i = 0; i < forbiden_devices_count; i++)
	{
		DXGI_ADAPTER_DESC1 desc1;
		adapter->lpVtbl->GetDesc1(adapter, &desc1);

		DXGI_ADAPTER_DESC1 desc2;
		Direct3D11Device* d3d11_device = D3D11_RETRIEVE_DRIVER_DATA_AS(forbiden_devices[i], Direct3D11Device*);
		d3d11_device->adapter->lpVtbl->GetDesc1(d3d11_device->adapter, &desc2);

		if(desc1.AdapterLuid.HighPart == desc2.AdapterLuid.HighPart && desc1.AdapterLuid.LowPart == desc2.AdapterLuid.LowPart)
			return true;
	}
	return false;
}

static IDXGIAdapter1* Direct3D11SelectAdapter(IDXGIFactory1* factory, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	IDXGIAdapter1* adapter = PULSE_NULLPTR;
	uint64_t best_device_score = 0;
	uint32_t best_device_id = 0;

	for(uint32_t i = 0; factory->lpVtbl->EnumAdapters1(factory, i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		if(Direct3D11IsAdapterForbidden(adapter, forbiden_devices, forbiden_devices_count))
			continue;
		uint64_t current_device_score = Direct3D11ScoreAdapter(adapter);
		if(current_device_score > best_device_score)
		{
			best_device_score = current_device_score;
			best_device_id = i;
		}
		adapter->lpVtbl->Release(adapter);
	}
	if(factory->lpVtbl->EnumAdapters1(factory, best_device_id, &adapter) == DXGI_ERROR_NOT_FOUND)
		return PULSE_NULLPTR;
	return adapter;
}

PulseDevice Direct3D11CreateDevice(PulseBackend backend, PulseDevice* forbiden_devices, uint32_t forbiden_devices_count)
{
	PULSE_CHECK_HANDLE_RETVAL(backend, PULSE_NULLPTR);

	PulseDevice pulse_device = (PulseDeviceHandler*)calloc(1, sizeof(PulseDeviceHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(pulse_device, PULSE_NULL_HANDLE);

	Direct3D11Device* device = (Direct3D11Device*)calloc(1, sizeof(Direct3D11Device));
	PULSE_CHECK_ALLOCATION_RETVAL(device, PULSE_NULL_HANDLE);

	pulse_device->driver_data = device;
	pulse_device->backend = backend;

	CreateDXGIFactory1(&IID_IDXGIFactory, (void**)&device->factory);

	device->adapter = Direct3D11SelectAdapter(device->factory, forbiden_devices, forbiden_devices_count);
	device->adapter->lpVtbl->GetDesc1(device->adapter, &device->description);

	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1, // :doubt:
		D3D_FEATURE_LEVEL_10_0
	};

	D3D11CreateDevice((IDXGIAdapter*)device->adapter, D3D_DRIVER_TYPE_UNKNOWN, PULSE_NULLPTR, 0, feature_levels, PULSE_SIZEOF_ARRAY(feature_levels), D3D11_SDK_VERSION, &device->device, PULSE_NULLPTR, &device->context);

	pulse_device->PFN_DestroyDevice = Direct3D11DestroyDevice;

	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(backend))
		PulseLogInfoFmt(backend, "(D3D11) created device from %ls", device->description.Description);
	return pulse_device;
}

void Direct3D11DestroyDevice(PulseDevice device)
{
	Direct3D11Device* d3d11_device = D3D11_RETRIEVE_DRIVER_DATA_AS(device, Direct3D11Device*);
	if(d3d11_device == PULSE_NULLPTR || d3d11_device->device == PULSE_NULLPTR)
		return;
	d3d11_device->device->lpVtbl->Release(d3d11_device->device);
	d3d11_device->context->lpVtbl->Release(d3d11_device->context);
	d3d11_device->adapter->lpVtbl->Release(d3d11_device->adapter);
	d3d11_device->factory->lpVtbl->Release(d3d11_device->factory);
	if(PULSE_IS_BACKEND_HIGH_LEVEL_DEBUG(device->backend))
		PulseLogInfoFmt(device->backend, "(D3D11) destroyed device created from %ls", d3d11_device->description.Description);
	free(d3d11_device);
	free(device);
}
