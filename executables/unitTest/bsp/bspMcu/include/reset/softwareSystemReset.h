/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif
uint32_t softwareSystemResetCalled;

void softwareSystemReset() { ++softwareSystemResetCalled; }

void softwareDestructiveReset(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
