// Copyright 2024 Accenture.

#include "busid/BusId.h"

namespace common
{
namespace busid
{
// clang-format off
#define BUS_ID_NAME(BUS) \
    case ::busid::BUS: return #BUS

// clang-format on

char const* BusIdTraits::getName(uint8_t index)
{
    switch (index)
    {
        BUS_ID_NAME(SELFDIAG);
        BUS_ID_NAME(CAN_0);
        default: return "INVALID";
    }
}

} // namespace busid
} // namespace common
