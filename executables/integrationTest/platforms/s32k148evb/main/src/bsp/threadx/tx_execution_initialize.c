/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/*
 * Custom profiling initialization function
 *
 * Do not remove! This is called by ThreadX core just before its scheduler runs
 * if TX_ENABLE_EXECUTION_CHANGE_NOTIFY is defined.
 */
void _tx_execution_initialize() { return; }
