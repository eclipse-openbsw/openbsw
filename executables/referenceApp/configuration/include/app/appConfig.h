// Copyright 2024 Accenture.

#pragma once

#include <async/TaskInitializer.h>

#ifdef PLATFORM_SUPPORT_UDS
static constexpr uint16_t LOGICAL_ADDRESS = 0x002AU;
#endif // PLATFORM_SUPPORT_UDS

#define safety_task_stackSize 2048U
