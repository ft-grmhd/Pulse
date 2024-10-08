// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <vulkan/vulkan_core.h>

#include <Pulse.h>
#include "../../PulseInternal.h"

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"

#ifdef PULSE_PLAT_MACOS
	#include <stdlib.h> // getenv
#endif

#ifdef PULSE_PLAT_WINDOWS
	typedef const char* LPCSTR;
	typedef struct HINSTANCE__* HINSTANCE;
	typedef HINSTANCE HMODULE;
	#if defined(_MINWINDEF_)
		/* minwindef.h defines FARPROC, and attempting to redefine it may conflict with -Wstrict-prototypes */
	#elif defined(_WIN64)
		typedef __int64 (__stdcall* FARPROC)(void);
	#else
		typedef int (__stdcall* FARPROC)(void);
	#endif
#endif

#ifdef PULSE_PLAT_WINDOWS
	__declspec(dllimport) HMODULE __stdcall LoadLibraryA(LPCSTR);
	__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE, LPCSTR);
	__declspec(dllimport) int __stdcall FreeLibrary(HMODULE);
	typedef HMODULE LibModule;
#else
	#include <dlfcn.h>
	typedef void* LibModule;
#endif

static LibModule vulkan_lib_module = PULSE_NULLPTR;

static bool VulkanLoadGlobalFunctions(void* context, PFN_vkVoidFunction (*load)(void*, const char*));
static bool VulkanLoadInstanceFunctions(VulkanInstance* instance, PFN_vkVoidFunction (*load)(void*, const char*));
static bool VulkanLoadDeviceFunctions(VulkanInstance* instance, VulkanDevice* device, PFN_vkVoidFunction (*load)(VulkanInstance*, void*, const char*));

static inline PFN_vkVoidFunction vkGetInstanceProcAddrStub(void* context, const char* name)
{
	PFN_vkVoidFunction fn = VulkanGetGlobal()->vkGetInstanceProcAddr((VkInstance)context, name);
	if(!fn)
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return fn;
}

static inline PFN_vkVoidFunction vkGetDeviceProcAddrStub(VulkanInstance* instance, void* context, const char* name)
{
	PFN_vkVoidFunction fn = instance->vkGetDeviceProcAddr((VkDevice)context, name);
	if(!fn)
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
	return fn;
}

static inline LibModule LoadLibrary(const char* libname)
{
	#ifdef PULSE_PLAT_WINDOWS
		return LoadLibraryA(libname);
	#else
		return dlopen(libname, RTLD_NOW | RTLD_LOCAL);
	#endif
}

static inline void* GetSymbol(LibModule module, const char* name)
{
	#ifdef PULSE_PLAT_WINDOWS
		return (void*)(void(*)(void))GetProcAddress(module, name);
	#else
		return dlsym(module, name);
	#endif
}

static inline void UnloadLibrary(LibModule lib)
{
	#ifdef PULSE_PLAT_WINDOWS
		FreeLibrary(lib);
	#else
		dlclose(lib);
	#endif
}

bool VulkanInitLoader()
{
	#if defined(PULSE_PLAT_WINDOWS)
		const char* libnames[] = {
			"vulkan-1.dll"
		};
	#elif defined(PULSE_PLAT_MACOS)
		const char* libnames[] = {
			"libvulkan.dylib",
			"libvulkan.1.dylib",
			"libMoltenVK.dylib",
			"vulkan.framework/vulkan",
			"MoltenVK.framework/MoltenVK",
			"/usr/local/lib/libvulkan.dylib",
		};
	#else
		const char* libnames[] = {
			"libvulkan.so.1",
			"libvulkan.so"
		};
	#endif

	for(size_t i = 0; i < sizeof(libnames) / sizeof(const char*); i++)
	{
		vulkan_lib_module = LoadLibrary(libnames[i]);
		if(vulkan_lib_module)
		{
			VulkanGetGlobal()->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetSymbol(vulkan_lib_module, "vkGetInstanceProcAddr");
			if(VulkanGetGlobal()->vkGetInstanceProcAddr == PULSE_NULLPTR)
			{
				UnloadLibrary(vulkan_lib_module);
				vulkan_lib_module = PULSE_NULLPTR;
			}
		}
	}
	if(!vulkan_lib_module)
	{
		PulseSetInternalError(PULSE_ERROR_INITIALIZATION_FAILED);
		return false;
	}
	return VulkanLoadGlobalFunctions(PULSE_NULLPTR, vkGetInstanceProcAddrStub);
}

bool VulkanLoadInstance(VulkanInstance* instance)
{
	return VulkanLoadInstanceFunctions(instance, vkGetInstanceProcAddrStub);
}

bool VulkanLoadDevice(VulkanInstance* instance, VulkanDevice* device)
{
	return VulkanLoadDeviceFunctions(instance, device, vkGetDeviceProcAddrStub);
}

void VulkanLoaderShutdown()
{
	UnloadLibrary(vulkan_lib_module);
	vulkan_lib_module = PULSE_NULLPTR;
}

static bool VulkanLoadGlobalFunctions(void* context, PFN_vkVoidFunction (*load)(void*, const char*))
{
	#define PULSE_VULKAN_GLOBAL_FUNCTION(func) \
		VulkanGetGlobal()->func = (PFN_##func)load(context, #func); \
		if(!VulkanGetGlobal()->func) \
			return false;
		#include "VulkanGlobalPrototypes.h"
	#undef PULSE_VULKAN_GLOBAL_FUNCTION
	return true;
}

static bool VulkanLoadInstanceFunctions(VulkanInstance* instance, PFN_vkVoidFunction (*load)(void*, const char*))
{
	#define PULSE_VULKAN_INSTANCE_FUNCTION(func) \
		instance->func = (PFN_##func)load(instance->instance, #func); \
		if(!instance->func) \
			return false;
		#include "VulkanInstancePrototypes.h"
	#undef PULSE_VULKAN_INSTANCE_FUNCTION
	return true;
}

static bool VulkanLoadDeviceFunctions(VulkanInstance* instance, VulkanDevice* device, PFN_vkVoidFunction (*load)(VulkanInstance*, void*, const char*))
{
	#define PULSE_VULKAN_DEVICE_FUNCTION(func) \
		device->func = (PFN_##func)load(instance, device->device, #func); \
		if(!device->func) \
			return false;
		#include "VulkanDevicePrototypes.h"
	#undef PULSE_VULKAN_DEVICE_FUNCTION
	return true;
}
