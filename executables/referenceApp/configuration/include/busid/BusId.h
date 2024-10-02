// Copyright 2024 Accenture.

#ifndef GUARD_BD852B35_5E85_4B02_B03F_F4B8E467DBF8
#define GUARD_BD852B35_5E85_4B02_B03F_F4B8E467DBF8

#include "common/busid/BusId.h"

#include <estd/optional.h>

#include <cstdint>

namespace busid
{
static constexpr uint8_t SELFDIAG = 1;
static constexpr uint8_t CAN_0    = 2;
static constexpr uint8_t LAST_BUS = CAN_0;

} // namespace busid

#endif /* GUARD_BD852B35_5E85_4B02_B03F_F4B8E467DBF8 */
