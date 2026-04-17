// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include <etl/print.h>
#include <util/stream/BspStubs.h>

extern "C" void etl_putchar(int c) { putByteToStdout(static_cast<uint8_t>(c)); }
