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

#include <etl/array.h>
#include <etl/limits.h>
#include <etl/span.h>
#include <cstddef>
#include <cstdint>

#include <middleware/core/MiddlewarePduBase.h>

namespace middleware::core
{

/**
 * \brief Fixed-size, sequence-locked PDU; see MiddlewarePduBase for the locking contract.
 *
 * \tparam SIZE The size of the PDU in bytes.
 */
template<size_t SIZE>
class MiddlewarePdu : private MiddlewarePduBase
{
public:
    MiddlewarePdu() { data_.fill(etl::numeric_limits<uint8_t>::max()); }

    void store(etl::span<uint8_t const, SIZE> const newData)
    {
        MiddlewarePduBase::store(newData, data_);
    }

    uintptr_t load(etl::span<uint8_t, SIZE> const buffer) const
    {
        return MiddlewarePduBase::load(buffer, data_);
    }

private:
    etl::array<uint8_t, SIZE> data_{};
};

} // namespace middleware::core
