// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/*
 * Custom profiling initialization function
 *
 * Do not remove! This is called by ThreadX core just before its scheduler runs
 * if TX_ENABLE_EXECUTION_CHANGE_NOTIFY is defined.
 */
void _tx_execution_initialize() { return; }
