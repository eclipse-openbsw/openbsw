// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace shed
{
enum class move_op : uint8_t
{
    SKIP,
    MOVE,
    MOVE_DONE,
    SKIP_DONE
};

inline move_op skip_if(bool const move) { return move ? move_op::SKIP : move_op::MOVE; }
} // namespace shed
