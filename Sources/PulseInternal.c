// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <string.h>

#include <PulseProfile.h>
#include "PulseInternal.h"

#ifndef PULSE_PLAT_WASM
	#include <tinycthread.h>

	PulseThreadID PulseGetThreadID()
	{
		return (PulseThreadID)thrd_current();
	}

	void PulseSleep(int32_t ms)
	{
		if(ms <= 0)
			return;
		thrd_sleep(&(struct timespec){ .tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000 }, PULSE_NULLPTR);
	}
#else
	#include <emscripten/threading.h>

	PulseThreadID PulseGetThreadID()
	{
		return (PulseThreadID)0;
	}

	void PulseSleep(int32_t ms)
	{
		if(ms <= 0)
			return;
		emscripten_thread_sleep(ms);
	}
#endif

#ifdef PULSE_PLAT_WINDOWS
	PULSE_IMPORT_API HMODULE __stdcall LoadLibraryA(LPCSTR);
	PULSE_IMPORT_API FARPROC __stdcall GetProcAddress(HMODULE, LPCSTR);
	PULSE_IMPORT_API int __stdcall FreeLibrary(HMODULE);
#else
	#ifdef PULSE_PLAT_MACOS
		#include <stdlib.h>
	#endif
	#include <dlfcn.h>
#endif

PulseLibModule PulseLoadLibrary(const char* libpath)
{
	PulseLibModule module;
	#ifdef PULSE_PLAT_WINDOWS
		module = LoadLibraryA(libpath);
	#else
		dlerror();
		module = dlopen(libpath, RTLD_NOW | RTLD_LOCAL);
	#endif
	return (module ? module : PULSE_NULL_LIB_MODULE);
}

PFN_PulseLibFunction PulseLoadSymbolFromLibModule(PulseLibModule module, const char* symbol)
{
	if(module == PULSE_NULL_LIB_MODULE)
		return PULSE_NULLPTR;
	PFN_PulseLibFunction function;
	#ifdef PULSE_PLAT_WINDOWS
		function = (PFN_PulseLibFunction)GetProcAddress(module, symbol);
	#else
		void* symbol_ptr = (PFN_PulseLibFunction)dlsym(module, symbol);
		*(void**)(&function) = symbol_ptr;
	#endif
	return function;
}

void PulseUnloadLibrary(PulseLibModule module)
{
	if(module == PULSE_NULL_LIB_MODULE)
		return;
	#ifdef PULSE_PLAT_WINDOWS
		FreeLibrary(module);
	#else
		dlclose(module);
	#endif
}

uint32_t PulseHashString(const char* str)
{
	uint32_t hash = 5381;
	for(size_t i = 0; i < strlen(str); i++)
		hash = ((hash << 5) + hash) + str[i];
	return hash;
}

uint32_t PulseHashCombine(uint32_t lhs, uint32_t rhs)
{
	lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
	return lhs;
}

size_t PulseStrlcpy(char* dst, const char* src, size_t maxlen)
{
	size_t srclen = strlen(src);
	if(maxlen > 0)
	{
		size_t len = (srclen < maxlen - 1 ? srclen : maxlen - 1);
		memcpy(dst, src, len);
		dst[len] = '\0';
	}
	return srclen;
}

char* PulseStrtokR(char* str, const char* delim, char** saveptr)
{
	if(str != PULSE_NULLPTR)
		*saveptr = str;
	else if(*saveptr == PULSE_NULLPTR)
		return PULSE_NULLPTR;
	char* token = strtok(*saveptr, delim);
	if(token != PULSE_NULLPTR)
		*saveptr = PULSE_NULLPTR;
	return token;
}

static int isspace(int x)
{
	return ((x) == ' ') || ((x) == '\t') || ((x) == '\r') || ((x) == '\n') || ((x) == '\f') || ((x) == '\v');
}

void PulseTrimString(char* str)
{
	char* start = str;
	while(*start && isspace((unsigned char)*start))
		start++;
	char* end = start + strlen(start) - 1;
	while(end > start && isspace((unsigned char)*end))
		end--;
	*(end + 1) = '\0';
	if(start != str)
		memmove(str, start, end - start + 2);
}
