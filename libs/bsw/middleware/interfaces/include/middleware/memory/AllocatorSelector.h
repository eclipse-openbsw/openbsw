// Copyright 2025 BMW AG

#pragma once

#include <etl/delegate.h>

#include <cstdint>

namespace middleware
{
namespace memory
{

using AllocateFunction          = etl::delegate<uint8_t*(uint32_t)>;
using AllocateSharedFunction    = etl::delegate<uint8_t*(uint32_t, uint8_t)>;
using DeallocateFunction        = etl::delegate<bool(uint8_t*)>;
using DeallocateSharedFunction  = etl::delegate<bool(uint8_t*, uint32_t)>;
using RegionStartFunction       = etl::delegate<uint8_t*()>;
using PointerValidationFunction = etl::delegate<bool(uint8_t const*)>;

/** Returns a function to allocate data for a service ID */
AllocateFunction getAllocFunction(uint16_t sid);

/** Returns a function to allocate shared data for a service ID */
AllocateSharedFunction getAllocSharedFunction(uint16_t sid);

/** Returns a function to deallocate data for a service ID */
DeallocateFunction getDeallocFunction(uint16_t sid);

/** Returns a function to deallocate shared data for a service ID */
DeallocateSharedFunction getDeallocSharedFunction(uint16_t sid);

/** Returns a function to get the region start for a service ID */
RegionStartFunction getRegionStartFunction(uint16_t sid);

/** Returns a function to validate a pointer for a service ID */
PointerValidationFunction getPtrValidationFunction(uint16_t sid);

} // namespace memory
} // namespace middleware
