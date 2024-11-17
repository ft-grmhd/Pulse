// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_PFNS_H_
#define PULSE_PFNS_H_

#include <Pulse.h>

typedef PulseBackendFlags (*PulseCheckBackendSupportPFN)(PulseBackendFlags, PulseShaderFormatsFlags);

typedef bool (*PulseLoadBackendPFN)(PulseDebugLevel);
typedef void (*PulseUnloadBackendPFN)(PulseBackend);
typedef PulseDevice (*PulseCreateDevicePFN)(PulseBackend, PulseDevice*, uint32_t);

typedef void (*PulseDestroyDevicePFN)(PulseDevice);
typedef PulseComputePipeline (*PulseCreateComputePipelinePFN)(PulseDevice, const PulseComputePipelineCreateInfo*);
typedef void (*PulseBindComputePipelinePFN)(PulseComputePipeline);
typedef void (*PulseDestroyComputePipelinePFN)(PulseDevice, PulseComputePipeline);
typedef PulseFence (*PulseCreateFencePFN)(PulseDevice);
typedef void (*PulseDestroyFencePFN)(PulseDevice, PulseFence);
typedef bool (*PulseIsFenceReadyPFN)(PulseDevice, PulseFence);
typedef bool (*PulseWaitForFencesPFN)(PulseDevice, const PulseFence*, uint32_t, bool);
typedef PulseCommandList (*PulseRequestCommandListPFN)(PulseDevice, PulseCommandListUsage);
typedef bool (*PulseSubmitCommandListPFN)(PulseDevice, PulseCommandList, PulseFence);
typedef void (*PulseReleaseCommandListPFN)(PulseDevice, PulseCommandList);

#endif // PULSE_PFNS_H_
