// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include "bsp/Uart.h"

#include <cstddef>

namespace bsp
{

enum class Uart::Id : size_t
{
    TERMINAL,
    INVALID,
};

static constexpr size_t NUMBER_OF_UARTS = static_cast<size_t>(Uart::Id::INVALID);

} // namespace bsp
