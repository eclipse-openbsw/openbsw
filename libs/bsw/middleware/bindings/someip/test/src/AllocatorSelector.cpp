/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/memory/AllocatorSelector.h"

#include <etl/array.h>

#include <cstddef>
#include <cstdint>

// The middleware leaves the payload allocator to the application; the tests use a few
// fixed-size blocks, each with its reference count.
namespace middleware::memory
{
namespace
{

constexpr size_t BLOCK_SIZE  = 256U;
constexpr size_t BLOCK_COUNT = 8U;

alignas(std::max_align_t)::etl::array<uint8_t, BLOCK_SIZE * BLOCK_COUNT> region{};
::etl::array<uint8_t, BLOCK_COUNT> references{};

/** Index of the block \p ptr starts, or BLOCK_COUNT if it starts none. */
size_t blockOf(uint8_t const* const ptr)
{
    size_t block = 0U;
    while ((block < BLOCK_COUNT) && (ptr != &region[block * BLOCK_SIZE]))
    {
        ++block;
    }
    return block;
}

uint8_t* allocateShared(uint32_t const size, uint8_t const referenceCount)
{
    if ((size > BLOCK_SIZE) || (referenceCount == 0U))
    {
        return nullptr;
    }
    for (size_t block = 0U; block < BLOCK_COUNT; ++block)
    {
        if (references[block] == 0U)
        {
            references[block] = referenceCount;
            return &region[block * BLOCK_SIZE];
        }
    }
    return nullptr;
}

uint8_t* allocate(uint32_t const size) { return allocateShared(size, 1U); }

bool isPtrValid(uint8_t const* const ptr)
{
    size_t const block = blockOf(ptr);
    return (block < BLOCK_COUNT) && (references[block] != 0U);
}

// A block that is already free is refused, so a second free fails instead of passing.
bool deallocateShared(uint8_t* const ptr, uint32_t)
{
    if (!isPtrValid(ptr))
    {
        return false;
    }
    --references[blockOf(ptr)];
    return true;
}

bool deallocate(uint8_t* const ptr)
{
    if (!isPtrValid(ptr))
    {
        return false;
    }
    references[blockOf(ptr)] = 0U;
    return true;
}

uint8_t* regionStart() { return region.data(); }

} // namespace

AllocateFunction getAllocFunction(uint16_t) { return AllocateFunction::create<&allocate>(); }

AllocateSharedFunction getAllocSharedFunction(uint16_t)
{
    return AllocateSharedFunction::create<&allocateShared>();
}

DeallocateFunction getDeallocFunction(uint16_t)
{
    return DeallocateFunction::create<&deallocate>();
}

DeallocateSharedFunction getDeallocSharedFunction(uint16_t)
{
    return DeallocateSharedFunction::create<&deallocateShared>();
}

RegionStartFunction getRegionStartFunction(uint16_t)
{
    return RegionStartFunction::create<&regionStart>();
}

PointerValidationFunction getPtrValidationFunction(uint16_t)
{
    return PointerValidationFunction::create<&isPtrValid>();
}

} // namespace middleware::memory
