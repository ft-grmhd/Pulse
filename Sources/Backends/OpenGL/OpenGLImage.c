// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#include "../../PulseInternal.h"
#include "OpenGL.h"
#include "OpenGLImage.h"
#include "OpenGLDevice.h"
#include "OpenGLCommandList.h"

static GLenum PulseImageFormatToGLInternalFormat[] = {
	GL_INVALID_ENUM,   // INVALID
	GL_R8,             // A8_UNORM
	GL_R8,             // R8_UNORM
	GL_RG8,            // R8G8_UNORM
	GL_RGBA8,          // R8G8B8A8_UNORM
	GL_R16,            // R16_UNORM
	GL_RG16,           // R16G16_UNORM
	GL_RGBA16,         // R16G16B16A16_UNORM
	GL_RGB10_A2,       // R10G10B10A2_UNORM
	GL_RGB565,         // B5G6R5_UNORM
	GL_RGB5_A1,        // B5G5R5A1_UNORM
	GL_RGBA4,          // B4G4R4A4_UNORM
	GL_BGRA8_EXT,      // B8G8R8A8_UNORM
	GL_INVALID_ENUM,   // BC1_UNORM
	GL_INVALID_ENUM,   // BC2_UNORM
	GL_INVALID_ENUM,   // BC3_UNORM
	GL_INVALID_ENUM,   // BC4_UNORM
	GL_INVALID_ENUM,   // BC5_UNORM
	GL_INVALID_ENUM,   // BC7_UNORM
	GL_INVALID_ENUM,   // BC6H_FLOAT
	GL_INVALID_ENUM,   // BC6H_UFLOAT
	GL_R8_SNORM,       // R8_SNORM
	GL_RG8_SNORM,      // R8G8_SNORM
	GL_RGBA8_SNORM,    // R8G8B8A8_SNORM
	GL_R16_SNORM,      // R16_SNORM
	GL_RG16_SNORM,     // R16G16_SNORM
	GL_RGBA16_SNORM,   // R16G16B16A16_SNORM
	GL_R16F,           // R16_FLOAT
	GL_RG16F,          // R16G16_FLOAT
	GL_RGBA16F,        // R16G16B16A16_FLOAT
	GL_R32F,           // R32_FLOAT
	GL_RG32F,          // R32G32_FLOAT
	GL_RGBA32F,        // R32G32B32A32_FLOAT
	GL_R11F_G11F_B10F, // R11G11B10_UFLOAT
	GL_R8UI,           // R8_UINT
	GL_RG8UI,          // R8G8_UINT
	GL_RGBA8UI,        // R8G8B8A8_UINT
	GL_R16UI,          // R16_UINT
	GL_RG16UI,         // R16G16_UINT
	GL_RGBA16UI,       // R16G16B16A16_UINT
	GL_R32UI,          // R32_UINT
	GL_RG32UI,         // R32G32_UINT
	GL_RGBA32UI,       // R32G32B32A32_UINT
	GL_R8I,            // R8_INT
	GL_RG8I,           // R8G8_INT
	GL_RGBA8I,         // R8G8B8A8_INT
	GL_R16I,           // R16_INT
	GL_RG16I,          // R16G16_INT
	GL_RGBA16I,        // R16G16B16A16_INT
	GL_R32I,           // R32_INT
	GL_RG32I,          // R32G32_INT
	GL_RGBA32I,        // R32G32B32A32_INT
};
PULSE_STATIC_ASSERT(PulseImageFormatToGLInternalFormat, PULSE_SIZEOF_ARRAY(PulseImageFormatToGLInternalFormat) == PULSE_IMAGE_FORMAT_MAX_ENUM);

static GLenum PulseImageTypeToGLTextureType[] = {
	GL_TEXTURE_2D,             //PULSE_IMAGE_TYPE_2D
	GL_TEXTURE_2D_ARRAY,       //PULSE_IMAGE_TYPE_2D_ARRAY
	GL_TEXTURE_3D,             //PULSE_IMAGE_TYPE_3D
	GL_TEXTURE_CUBE_MAP,       //PULSE_IMAGE_TYPE_CUBE
	GL_TEXTURE_CUBE_MAP_ARRAY, //PULSE_IMAGE_TYPE_CUBE_ARRAY
};
PULSE_STATIC_ASSERT(PulseImageTypeToGLTextureType, PULSE_SIZEOF_ARRAY(PulseImageTypeToGLTextureType) == PULSE_IMAGE_TYPE_MAX_ENUM);

PulseImage OpenGLCreateImageTryAndFail(PulseDevice device, const PulseImageCreateInfo* create_infos, bool try_and_fail)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);

	PulseImageHandler* image = (PulseImageHandler*)calloc(1, sizeof(PulseImageHandler));
	PULSE_CHECK_ALLOCATION_RETVAL(image, PULSE_NULL_HANDLE);

	OpenGLImage* opengl_image = (OpenGLImage*)calloc(1, sizeof(OpenGLImage));
	PULSE_CHECK_ALLOCATION_RETVAL(opengl_image, PULSE_NULL_HANDLE);

	GLenum image_type = PulseImageTypeToGLTextureType[create_infos->type];

	// TODO error message if image_type is invalid

	opengl_device->glGenTextures(device, 1, &opengl_image->image);
	opengl_device->glBindTexture(device, image_type, opengl_image->image);
	if(try_and_fail)
	{
		while(((PFNGLGETERRORPROC)opengl_device->original_function_ptrs[glGetError])() != GL_NO_ERROR); // Clear errors
		((PFNGLTEXSTORAGE2DPROC)opengl_device->original_function_ptrs[glTexStorage2D])();
	}
	else
	{
		if(create_infos->type == PULSE_IMAGE_TYPE_3D)
			opengl_device->glTexStorage3D(device, image_type, 1, PulseImageFormatToGLInternalFormat[create_infos->format], create_infos->width, create_infos->width, create_infos->layer_count_or_depth);
		else
			opengl_device->glTexStorage2D(device, image_type, 1, PulseImageFormatToGLInternalFormat[create_infos->format], create_infos->width, create_infos->height);
	}
	if(create_infos->format == PULSE_IMAGE_FORMAT_A8_UNORM)
	{
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_A, GL_RED);
	}
	else if(create_infos->format == PULSE_IMAGE_FORMAT_B4G4R4A4_UNORM)
	{
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_G, GL_RED);
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_B, GL_ALPHA);
		opengl_device->glTexParameteri(device, image_type, GL_TEXTURE_SWIZZLE_A, GL_BLUE);
	}

	opengl_device->glBindTexture(device, image_type, 0); // Unbind

	return image;
}

PulseImage OpenGLCreateImage(PulseDevice device, const PulseImageCreateInfo* create_infos)
{
	return OpenGLCreateImageTryAndFail(device, create_infos, false);
}

bool OpenGLIsImageFormatValid(PulseDevice device, PulseImageFormat format, PulseImageType type, PulseImageUsageFlags usage)
{
	OpenGLDevice* opengl_device = OPENGL_RETRIEVE_DRIVER_DATA_AS(device, OpenGLDevice*);
}

bool OpenGLCopyImageToBuffer(PulseCommandList cmd, const PulseImageRegion* src, const PulseBufferRegion* dst)
{
}

bool OpenGLBlitImage(PulseCommandList cmd, const PulseImageRegion* src, const PulseImageRegion* dst)
{
}

void OpenGLDestroyImage(PulseDevice device, PulseImage image)
{
	OpenGLImage* opengl_image = OPENGL_RETRIEVE_DRIVER_DATA_AS(image, OpenGLImage*);
	free(opengl_image);
	free(image);
}
