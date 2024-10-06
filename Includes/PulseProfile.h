// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef PULSE_PROFILE_H_
#define PULSE_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PULSE_DEFINE_NULLABLE_HANDLE(object) typedef struct object##Handler* object

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201103L))
	#define PULSE_NULL_HANDLE nullptr
#else
	#define PULSE_NULL_HANDLE ((void*)0)
#endif

#define PULSE_MAKE_VERSION(major, minor, patch) ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))

#define PULSE_VERSION_MAJOR(version) (((uint32_t)(version) >> 22U) & 0x7FU)
#define PULSE_VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define PULSE_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

#define PULSE_VERSION_1_0 PULSE_MAKE_VERSION(1, 0, 0)

#define PULSE_BIT(x) (1u << x)

#ifdef __cplusplus
}
#endif

#endif // PULSE_PROFILE_H_
