// Copyright 2025 Accenture.

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
