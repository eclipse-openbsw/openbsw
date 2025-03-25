// Copyright 2024 Accenture.

#pragma once

#include "common/busid/BusId.h"

#include <estd/optional.h>

#include <cstdint>

namespace busid
{
static constexpr uint8_t SELFDIAG = 1;
static constexpr uint8_t CAN_0    = 2;
static constexpr uint8_t LAST_BUS = CAN_0;

} // namespace busid
