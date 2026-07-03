/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

#include <cstdint>
#include <cstdlib>

namespace middleware::shm
{

extern "C" uint8_t* gShmAddr;
extern "C" size_t gShmSize;

class ShmWrapper
{
private:
    int shm_fd_{-1}; // -1 = invalid / not yet opened

public:
    ShmWrapper();
    ~ShmWrapper();

    void init();
    void deInit();
};

/**
 * \brief Constructs the MemoryLayout in the shared memory region via placement new.
 * \details Must be called exactly once, by the startup core (core0 equivalent), after
 *          ShmWrapper::init() has mapped the shared memory. Corresponds to the
 *          ETL_CONSTINIT typed_storage<MemoryLayout>.create() pattern used on hardware.
 */
void createMemoryLayout();

} // namespace middleware::shm
