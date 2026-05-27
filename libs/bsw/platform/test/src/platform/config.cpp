/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "platform/config.h"

namespace
{
ESR_PACKED_BEGIN

struct A
{
    int a : 2;
    int b : 6;
};

ESR_PACKED_END

struct B
{
    int a : 2;
    int b : 6;
};

static_assert(sizeof(A) == 1, "ESR_PACKED_BEGIN should work");
static_assert(sizeof(B) != 1, "ESR_PACKED_END should work");

} // namespace
