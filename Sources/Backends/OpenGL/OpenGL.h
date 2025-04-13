// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_OPENGL_BACKEND

#ifndef PULSE_OPENGL_H_
#define PULSE_OPENGL_H_

#define OPENGL_RETRIEVE_DRIVER_DATA_AS(handle, cast) ((cast)handle->driver_data)

#include "OpenGLEnums.h"

#define GL_GLES_PROTOTYPES 0
#include <GLES3/gl32.h>

#define GL_R8 0x8229
#define GL_R16 0x822A
#define GL_RG8 0x822B
#define GL_RG16 0x822C
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#define GL_RGBA16 0x805B
#define GL_BGRA8_EXT 0x93A1
#define GL_R16_SNORM 0x8F98
#define GL_RG16_SNORM 0x8F99
#define GL_RGB16_SNORM 0x8F9A
#define GL_RGBA16_SNORM 0x8F9B

typedef void(*GLFunction)(void);

PulseBackendFlags OpenGLCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_OPENGL in case of success and PULSE_BACKEND_INVALID otherwise
PulseBackendFlags OpenGLESCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_OPENGL_ES in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_OPENGL_H_

#endif // PULSE_ENABLE_OPENGL_BACKEND
