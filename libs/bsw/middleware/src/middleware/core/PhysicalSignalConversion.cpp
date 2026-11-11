/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/PhysicalSignalConversion.h"

#include <etl/limits.h>
#include <etl/optional.h>
#include <cmath>
#include <cstdint>

namespace middleware::core
{

etl::optional<uint64_t> physicalToRaw(
    double const value, double const scalingFactor, double const offset, uint8_t const bitLength)
{
    constexpr uint8_t rawBitCount = etl::numeric_limits<uint64_t>::digits;
    if (std::isnan(value) || !std::isfinite(scalingFactor) || scalingFactor == 0.0
        || !std::isfinite(offset) || bitLength == 0U || bitLength > rawBitCount)
    {
        return etl::nullopt;
    }

    double const raw = (value - offset) / scalingFactor;
    if (raw <= 0.0)
    {
        return uint64_t{0U};
    }
    if (raw >= std::ldexp(1.0, bitLength))
    {
        return etl::numeric_limits<uint64_t>::max() >> (rawBitCount - bitLength);
    }
    return static_cast<uint64_t>(raw);
}

} // namespace middleware::core
