// Copyright 2024 Accenture.

#pragma once

#include <app/appConfig.h>

#include <platform/estdint.h>

// #include <portmacro.h>

extern uint32_t const __MPU_BSS_START[];
extern uint32_t const __MPU_BSS_END[];
extern ::async::internal::Stack<safety_task_stackSize> safetyStack;
