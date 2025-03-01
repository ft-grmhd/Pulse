// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <stdatomic.h>

#include <Pulse.h>
#include "WebGPU.h"
#include "WebGPUDevice.h"
#include "WebGPUFence.h"
#include "WebGPUCommandList.h"
#include "WebGPUComputePass.h"
#include "../../PulseInternal.h"

PulseCommandList WebGPURequestCommandList(PulseDevice device, PulseCommandListUsage usage)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);

	PULSE_UNUSED(usage);

	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);

	PulseCommandList cmd = (PulseCommandList)calloc(1, sizeof(PulseCommandListHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(cmd, PULSE_NULL_HANDLE);

	WebGPUCommandList* webgpu_cmd = (WebGPUCommandList*)calloc(1, sizeof(WebGPUCommandList));
	PULSE_CHECK_ALLOCATION_RETVAL(webgpu_cmd, PULSE_NULL_HANDLE);

	WGPUCommandEncoderDescriptor encoder_descriptor = { 0 };
	webgpu_cmd->encoder = wgpuDeviceCreateCommandEncoder(webgpu_device->device, &encoder_descriptor);

	cmd->usage = usage;
	cmd->device = device;
	cmd->driver_data = webgpu_cmd;
	cmd->thread_id = PulseGetThreadID();

	cmd->pass = WebGPUCreateComputePass(device, cmd);
	cmd->state = PULSE_COMMAND_LIST_STATE_RECORDING;
	cmd->is_available = false;

	return cmd;
}

static void WebGPUFenceCallback(WGPUQueueWorkDoneStatus status, void* userdata1, void* userdata2)
{
	PULSE_UNUSED(userdata2);
	WebGPUFence* webgpu_fence = (WebGPUFence*)userdata1;
	PulseCommandList cmd = (PulseCommandList)userdata2;
	if(status == WGPUQueueWorkDoneStatus_Success)
	{
		if(webgpu_fence != PULSE_NULLPTR)
			atomic_store(&webgpu_fence->signal, true);
		cmd->state = PULSE_COMMAND_LIST_STATE_READY;
	}
}

bool WebGPUSubmitCommandList(PulseDevice device, PulseCommandList cmd, PulseFence fence)
{
	WebGPUDevice* webgpu_device = WEBGPU_RETRIEVE_DRIVER_DATA_AS(device, WebGPUDevice*);
	WebGPUCommandList* webgpu_cmd = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd, WebGPUCommandList*);

	WGPUCommandBufferDescriptor command_buffer_descriptor = { 0 };
	WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(webgpu_cmd->encoder, &command_buffer_descriptor);

	WGPUQueueWorkDoneCallbackInfo callback = { 0 };
	callback.mode = WGPUCallbackMode_AllowSpontaneous;
	callback.callback = WebGPUFenceCallback;
	callback.userdata1 = PULSE_NULLPTR;
	callback.userdata2 = cmd;
	if(fence != PULSE_NULL_HANDLE)
	{
		WebGPUFence* webgpu_fence = WEBGPU_RETRIEVE_DRIVER_DATA_AS(fence, WebGPUFence*);
		callback.userdata1 = webgpu_fence;
		atomic_store(&webgpu_fence->signal, false);
		fence->cmd = cmd;
	}
	wgpuQueueOnSubmittedWorkDone(webgpu_device->queue, callback);

	cmd->state = PULSE_COMMAND_LIST_STATE_SENT;
	wgpuQueueSubmit(webgpu_device->queue, 1, &command_buffer);

	wgpuCommandBufferRelease(command_buffer);
	return true;
}

void WebGPUReleaseCommandList(PulseDevice device, PulseCommandList cmd)
{
	PULSE_CHECK_HANDLE(device);

	WebGPUCommandList* webgpu_cmd = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd, WebGPUCommandList*);
	WebGPUComputePass* webgpu_pass = WEBGPU_RETRIEVE_DRIVER_DATA_AS(cmd->pass, WebGPUComputePass*);

	wgpuCommandEncoderRelease(webgpu_cmd->encoder);

	free(webgpu_pass);
	free(cmd->pass);
	free(webgpu_cmd);
	free(cmd);
}
