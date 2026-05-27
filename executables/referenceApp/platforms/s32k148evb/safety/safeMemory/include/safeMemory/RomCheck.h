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

#include <platform/estdint.h>

namespace safety
{

class RomCheck
{
public:
    enum class Result : uint8_t
    {
        ROMCHECK_REQUESTED = 0,
        ROMCHECK_OK        = 1,
        ROMCHECK_FAILED    = 2
    };

    static void init();
    static void idle();

    static Result result() { return _result; }

private:
    static Result _result;
};

} // namespace safety
