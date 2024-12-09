// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// conditions of distribution and use, see copyright notice in LICENSE

#include "Pulse.h"
#include "PulseDefs.h"
#include "PulseInternal.h"

PULSE_API PulseImage PulseCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos)
{
	PULSE_CHECK_HANDLE_RETVAL(device, PULSE_NULL_HANDLE);
	if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
	{
		if(create_infos == PULSE_NULLPTR)
		{
			PulseLogError(device->backend, "create_infos is NULL");
			PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
			return PULSE_NULL_HANDLE;
		}

		bool failed = false;
		const uint32_t MAX_2D_DIMENSION = 16384;
		const uint32_t MAX_3D_DIMENSION = 2048;

		if(create_infos->width <= 0 || create_infos->height <= 0 || create_infos->layer_count_or_depth <= 0)
		{
			PulseLogError(device->backend, "any image: width, height, and layer_count_or_depth must be >= 1");
			failed = true;
		}

		if(create_infos->type == PULSE_IMAGE_TYPE_CUBE)
		{
			if(create_infos->width != create_infos->height)
			{
				PulseLogError(device->backend, "cube images: width and height must be identical");
				failed = true;
			}
			if(create_infos->width > MAX_2D_DIMENSION || create_infos->height > MAX_2D_DIMENSION)
			{
				PulseLogError(device->backend, "cube images: width and height must be <= 16384");
				failed = true;
			}
			if(create_infos->layer_count_or_depth != 6)
			{
				PulseLogError(device->backend, "cube images: layer_count_or_depth must be 6");
				failed = true;
			}
			if(!PulseIsImageFormatValid(device, create_infos->format, PULSE_IMAGE_TYPE_CUBE, create_infos->usage))
			{
				PulseLogError(device->backend, "cube images: the format is unsupported for the given usage");
				failed = true;
			}
		}
		else if(create_infos->type == PULSE_IMAGE_TYPE_CUBE_ARRAY)
		{
			if(create_infos->width != create_infos->height)
			{
				PulseLogError(device->backend, "cube array images: width and height must be identical");
				failed = true;
			}
			if(create_infos->width > MAX_2D_DIMENSION || create_infos->height > MAX_2D_DIMENSION)
			{
				PulseLogError(device->backend, "cube array images: width and height must be <= 16384");
				failed = true;
			}
			if(create_infos->layer_count_or_depth % 6 != 0)
			{
				PulseLogError(device->backend, "cube array images: layer_count_or_depth must be a multiple of 6");
				failed = true;
			}
			if(!PulseIsImageFormatValid(device, create_infos->format, PULSE_IMAGE_TYPE_CUBE_ARRAY, create_infos->usage))
			{
				PulseLogError(device->backend, "cube array images: the format is unsupported for the given usage");
				failed = true;
			}
		}
		else if(create_infos->type == PULSE_IMAGE_TYPE_3D)
		{
			if(create_infos->width > MAX_3D_DIMENSION || create_infos->height > MAX_3D_DIMENSION || create_infos->layer_count_or_depth > MAX_3D_DIMENSION)
			{
				PulseLogError(device->backend, "3D images: width, height, and layer_count_or_depth must be <= 2048");
				failed = true;
			}
			if(!PulseIsImageFormatValid(device, create_infos->format, PULSE_IMAGE_TYPE_3D, create_infos->usage))
			{
				PulseLogError(device->backend, "3D images: the format is unsupported for the given usage");
				failed = true;
			}
		}
		else
		{
			if(!PulseIsImageFormatValid(device, create_infos->format, PULSE_IMAGE_TYPE_2D, create_infos->usage))
			{
				PulseLogError(device->backend, "2D images: the format is unsupported for the given usage");
				failed = true;
			}
		}

		if(failed)
			return PULSE_NULL_HANDLE;
	}
	return device->PFN_CreateImage(device, create_infos);
}

PULSE_API bool PulseIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage)
{
	PULSE_CHECK_HANDLE_RETVAL(device, false);
	return device->PFN_IsImageFormatValid(device, format, type, usage);
}

PULSE_API void PulseDestroyImage(PulseDevice device, PulseImage image)
{
	PULSE_CHECK_HANDLE(device);

	if(image == PULSE_NULL_HANDLE)
	{
		if(PULSE_IS_BACKEND_LOW_LEVEL_DEBUG(device->backend))
			PulseLogWarning(device->backend, "image is NULL, this may be a bug in your application");
		return;
	}
	return device->PFN_DestroyImage(device, image);
}