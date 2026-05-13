// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include <stdint.h>

// Redefine SYS functions for NVIC access (used by startup/ISR setup)
#define SYS_SetPriority(irq, prio) NVIC_SetPriority((irq), (prio))
#define SYS_EnableIRQ(irq)         NVIC_EnableIRQ((irq))
#define SYS_DisableIRQ(irq)        NVIC_DisableIRQ((irq))
