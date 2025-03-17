// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Pulse.h>

#ifdef PULSE_ENABLE_SOFTWARE_BACKEND

#ifndef PULSE_SOFTWARE_H_
#define PULSE_SOFTWARE_H_

#ifdef __STDC_NO_ATOMICS__
	#error "Atomic support is not present"
#endif

#define SOFT_RETRIEVE_DRIVER_DATA_AS(handle, cast) ((cast)handle->driver_data)

PulseBackendFlags SoftCheckSupport(PulseBackendFlags candidates, PulseShaderFormatsFlags shader_formats_used); // Return PULSE_BACKEND_SOFTWARE in case of success and PULSE_BACKEND_INVALID otherwise

#endif // PULSE_SOFTWARE_H_

#endif // PULSE_ENABLE_SOFTWARE_BACKEND
