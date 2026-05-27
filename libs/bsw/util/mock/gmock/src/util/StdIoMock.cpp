/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/StdIoMock.h"

using namespace ::util::test;

extern "C"
{
int32_t getByteFromStdin()
{
    auto& in          = StdIoMock::instance().in;
    int32_t const ret = in.size() != 0 ? in[0] : -1;
    in.advance(1);
    return ret;
}

void putByteToStdout(uint8_t b) { StdIoMock::instance().out.push_back(b); }
} /* extern "C" */
