// Copyright 2024 Accenture.

#include "common/mock/busid/BusId.h"
using namespace std;

namespace common
{
namespace busid
{

#define BUS_ID_NAME(BUS) \
    case ::common::mock::busid::BUS: return #BUS

char const* BusIdTraits::getName(uint8_t index)
{
    switch (index)
    {
        BUS_ID_NAME(BUS0);
        BUS_ID_NAME(BUS1);
        BUS_ID_NAME(BUS2);
        default: return "INVALID";
    }
}

} // namespace busid
} // namespace common
