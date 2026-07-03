/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
/**
  *
  *  __  __ _     _     _ _
  * |  \/  (_) __| | __| | | _____      ____ _ _ __ ___
  * | |\/| | |/ _` |/ _` | |/ _ \ \ /\ / / _` | '__/ _ \
  * | |  | | | (_| | (_| | |  __/\ V  V / (_| | | |  __/
  * |_|  |_|_|\__,_|\__,_|_|\___| \_/\_/ \__,_|_|  \___|
  *
  * WARNING!
  * This file is generated. Do not edit manually.
  *
  */
#include <etl/type_traits.h>
#include <middleware/memory/AllocatorSelector.h>

#include "middleware/core/types.h"
#include "shm/AllocatorsDefinitions.h"


namespace middleware::memory
{

AllocateFunction getAllocFunction(uint16_t sid)
{
    switch (sid)
    {
        default: {
            using Base = etl::remove_reference_t<decltype(shm::getDefaultAllocator())>::Base;
            return AllocateFunction::create<Base, &Base::allocate>(shm::getDefaultAllocator());
        }
    }
}

AllocateSharedFunction getAllocSharedFunction(uint16_t sid)
{
    switch (sid)
    {
        default: {
            using Base = etl::remove_reference_t<decltype(shm::getDefaultAllocator())>::Base;
            return AllocateSharedFunction::create<Base, &Base::allocateShared>(shm::getDefaultAllocator());
        }
    }
}

DeallocateFunction getDeallocFunction(uint16_t sid)
{
    switch (sid)
    {
        default: {
            using Base = etl::remove_reference_t<decltype(shm::getDefaultAllocator())>::Base;
            return DeallocateFunction::create<Base, &Base::deallocate>(shm::getDefaultAllocator());
        }
    }
}

DeallocateSharedFunction getDeallocSharedFunction(uint16_t sid)
{
    switch (sid)
    {
        default: {
            using Base = etl::remove_reference_t<decltype(shm::getDefaultAllocator())>::Base;
            return DeallocateSharedFunction::create<Base, &Base::deallocateShared>(shm::getDefaultAllocator());
        }
    }
}

RegionStartFunction getRegionStartFunction(uint16_t sid)
{
    switch (sid)
    {
        default: {
            using Base = etl::remove_reference_t<decltype(shm::getDefaultAllocator())>::Base;
            return RegionStartFunction::create<Base, &Base::regionStart>(shm::getDefaultAllocator());
        }
    }
}

PointerValidationFunction getPtrValidationFunction(uint16_t sid)
{
    switch (sid)
    {
        default: {
            using Base = etl::remove_reference_t<decltype(shm::getDefaultAllocator())>::Base;
            return PointerValidationFunction::create<Base, &Base::isPtrValid>(shm::getDefaultAllocator());
        }
    }
}

}  // namespace middleware::memory
