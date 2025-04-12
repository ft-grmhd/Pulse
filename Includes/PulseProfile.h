// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef PULSE_PROFILE_H_
#define PULSE_PROFILE_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__BORLANDC__)
	#define PULSE_COMPILER_BORDLAND
#elif defined(__clang__)
	#define PULSE_COMPILER_CLANG
	#ifdef __MINGW32__
		#define PULSE_COMPILER_MINGW
		#ifdef __MINGW64_VERSION_MAJOR
			#define PULSE_COMPILER_MINGW_W64
		#endif
	#endif
#elif defined(__GNUC__) || defined(__MINGW32__)
	#define PULSE_COMPILER_GCC
	#ifdef __MINGW32__
		#define PULSE_COMPILER_MINGW
		#ifdef __MINGW64_VERSION_MAJOR
			#define PULSE_COMPILER_MINGW_W64
		#endif
	#endif
#elif defined(__INTEL_COMPILER) || defined(__ICL)
	#define PULSE_COMPILER_INTEL
#elif defined(_MSC_VER)
	#define PULSE_COMPILER_MSVC
#elif __EMSCRIPTEN__
	#define PULSE_COMPILER_EMSCRIPTEN
#else
	#define PULSE_COMPILER_UNKNOWN
	#warning "This compiler is not fully supported"
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
	#define PULSE_PLAT_WINDOWS
#elif defined(__linux__)
	#define PULSE_PLAT_LINUX
	#define PULSE_PLAT_POSIX
#elif defined(__APPLE__) && defined(__MACH__)
	#define PULSE_PLAT_MACOS
	#define PULSE_PLAT_POSIX
#elif defined(unix) || defined(__unix__) || defined(__unix)
	#define PULSE_PLAT_UNIX
	#define PULSE_PLAT_POSIX
#else
	#error "Unknown environment (not Windows, not Linux, not MacOS, not Unix)"
#endif

#ifndef PULSE_STATIC
	#ifdef PULSE_PLAT_WINDOWS
		#ifdef PULSE_COMPILER_MSVC
			#ifdef PULSE_BUILD
				#define PULSE_API __declspec(dllexport)
			#else
				#define PULSE_API __declspec(dllimport)
			#endif
		#elif defined(PULSE_COMPILER_GCC)
			#ifdef PULSE_BUILD
				#define PULSE_API __attribute__((dllexport))
			#else
				#define PULSE_API __attribute__((dllimport))
			#endif
		#else
			#define PULSE_API
		#endif
	#elif defined(PULSE_COMPILER_GCC)
		#define PULSE_API __attribute__((visibility("default")))
	#else
		#define PULSE_API
	#endif
#else
	#define PULSE_API
#endif

#ifdef PULSE_COMPILER_MSVC
	#define PULSE_IMPORT_API __declspec(dllimport)
#elif defined(PULSE_COMPILER_MINGW)
	#define PULSE_IMPORT_API __attribute__((dllimport))
#else
	#define PULSE_IMPORT_API
#endif

#ifndef __cplusplus // if we compile in C
	#ifdef __STDC__
		#ifdef __STDC_VERSION__
			#if __STDC_VERSION__ == 199409L
				#define PULSE_C_VERSION 1994
			#elif __STDC_VERSION__ == 199901L
				#define PULSE_C_VERSION 1999
			#elif __STDC_VERSION__ == 201112L
				#define PULSE_C_VERSION 2011
			#elif __STDC_VERSION__ == 201710L
				#define PULSE_C_VERSION 2017
			#elif __STDC_VERSION__ == 202311L
				#define PULSE_C_VERSION 2023
			#else
				#define PULSE_C_VERSION 0
			#endif
		#else
			#define PULSE_C_VERSION 0
		#endif
	#else
		#define PULSE_C_VERSION 0
	#endif
#else
	#define PULSE_C_VERSION 0
#endif

#if PULSE_C_VERSION >= 2023
	#if defined(PULSE_COMPILER_GCC) || defined(PULSE_COMPILER_CLANG) // for now only GCC and Clang supports nullptr
		#define PULSE_NULLPTR nullptr
	#else
		#define PULSE_NULLPTR NULL
	#endif
#elif defined(__cplusplus) && __cplusplus >= 201103L
	#define PULSE_NULLPTR nullptr
#else
	#define PULSE_NULLPTR NULL
#endif

#define PULSE_DEFINE_NULLABLE_HANDLE(object) typedef struct object##Handler* object

#define PULSE_NULL_HANDLE PULSE_NULLPTR

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
