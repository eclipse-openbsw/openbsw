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

/**
 * Asynchronous buffer size for logger output in bytes
 */
#ifndef __DEBUG_ANIMATION
#define CHARIOSERIAL_BUFFERSIZE 256
#else
#define CHARIOSERIAL_BUFFERSIZE 100
#endif

/**
 * Timeout for busy wait
 */
#define SCI_LOGGERTIMEOUT 10000
