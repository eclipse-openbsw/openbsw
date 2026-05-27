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

/**
 * Buffer size in bytes for asynchronous logger output
 */
#define CHARIOSERIAL_BUFFERSIZE 400

/**
 * Timeout for waiting serial console to get ready for sending,
 * in number of while-loop iterations
 */
#define SCI_LOGGERTIMEOUT 10000
