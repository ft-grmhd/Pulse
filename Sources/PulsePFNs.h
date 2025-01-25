// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_PFNS_H_
#define PULSE_PFNS_H_

#include <Pulse.h>

typedef PulseBackendFlags (*PulseCheckBackendSupportPFN)(PulseBackendFlags, PulseShaderFormatsFlags);

typedef bool (*PulseLoadBackendPFN)(PulseBackend, PulseDebugLevel);
typedef void (*PulseUnloadBackendPFN)(PulseBackend);
typedef PulseDevice (*PulseCreateDevicePFN)(PulseBackend, PulseDevice*, uint32_t);

typedef void (*PulseDestroyDevicePFN)(PulseDevice);
typedef PulseComputePipeline (*PulseCreateComputePipelinePFN)(PulseDevice, const PulseComputePipelineCreateInfo*);
typedef void (*PulseDispatchComputationsPFN)(PulseComputePass, uint32_t, uint32_t, uint32_t);
typedef void (*PulseDestroyComputePipelinePFN)(PulseDevice, PulseComputePipeline);
typedef PulseFence (*PulseCreateFencePFN)(PulseDevice);
typedef void (*PulseDestroyFencePFN)(PulseDevice, PulseFence);
typedef bool (*PulseIsFenceReadyPFN)(PulseDevice, PulseFence);
typedef bool (*PulseWaitForFencesPFN)(PulseDevice, const PulseFence*, uint32_t, bool);
typedef PulseCommandList (*PulseRequestCommandListPFN)(PulseDevice, PulseCommandListUsage);
typedef bool (*PulseSubmitCommandListPFN)(PulseDevice, PulseCommandList, PulseFence);
typedef void (*PulseReleaseCommandListPFN)(PulseDevice, PulseCommandList);
typedef PulseBuffer (*PulseCreateBufferPFN)(PulseDevice, const PulseBufferCreateInfo*);
typedef bool (*PulseMapBufferPFN)(PulseBuffer, void**);
typedef void (*PulseUnmapBufferPFN)(PulseBuffer);
typedef void (*PulseDestroyBufferPFN)(PulseDevice, PulseBuffer);
typedef PulseImage (*PulseCreateImagePFN)(PulseDevice, const PulseImageCreateInfo*);
typedef bool (*PulseIsImageFormatValidPFN)(PulseDevice, PulseImageFormat, PulseImageType, PulseImageUsageFlags);
typedef void (*PulseDestroyImagePFN)(PulseDevice, PulseImage);
typedef bool (*PulseCopyBufferToBufferPFN)(PulseCommandList, const PulseBufferRegion*, const PulseBufferRegion*);
typedef bool (*PulseCopyBufferToImageFN)(PulseCommandList, const PulseBufferRegion*, const PulseImageRegion*);
typedef bool (*PulseCopyImageToBufferPFN)(PulseCommandList, const PulseImageRegion*, const PulseBufferRegion*);
typedef bool (*PulseBlitImagePFN)(PulseCommandList, const PulseImageRegion*, const PulseImageRegion*);
typedef PulseComputePass (*PulseBeginComputePassPFN)(PulseCommandList);
typedef void (*PulseBindStorageBuffersPFN)(PulseComputePass, uint32_t, PulseBuffer* const*, uint32_t);
typedef void (*PulseBindUniformDataPFN)(PulseComputePass, uint32_t, const void*, uint32_t);
typedef void (*PulseBindStorageImagesPFN)(PulseComputePass, uint32_t, PulseImage* const*, uint32_t);
typedef void (*PulseBindComputePipelinePFN)(PulseComputePass, PulseComputePipeline);
typedef void (*PulseEndComputePassPFN)(PulseComputePass);

#endif // PULSE_PFNS_H_
