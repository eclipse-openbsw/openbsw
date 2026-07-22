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
