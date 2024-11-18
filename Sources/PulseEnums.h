// Copyright (C) 2024 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef PULSE_ENUMS_H_
#define PULSE_ENUMS_H_

typedef enum PulseCommandListState
{
	PULSE_COMMAND_LIST_STATE_INVALID,
	PULSE_COMMAND_LIST_STATE_RECORDING,
	PULSE_COMMAND_LIST_STATE_READY,
	PULSE_COMMAND_LIST_STATE_SENT
} PulseCommandListState;

#endif
