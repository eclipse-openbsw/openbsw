/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

extern "C"
{
void _tx_execution_thread_enter();

void _tx_execution_thread_exit();

void tx_low_power_enter();

void tx_low_power_exit();
}
