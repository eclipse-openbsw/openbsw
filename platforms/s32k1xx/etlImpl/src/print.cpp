/********************************************************************************
 * Copyright (c) 2026 BMW
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/


#include <etl/print.h>
#include <util/stream/BspStubs.h>

extern "C" void etl_putchar(int c) { putByteToStdout(static_cast<uint8_t>(c)); }
