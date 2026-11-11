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

#include <etl/optional.h>
#include <cstdint>

namespace middleware::core
{

/**
 * \brief Converts a physical value without risking an out-of-range unsigned cast.
 *
 * Saturates to the unsigned wire range and truncates fractional raw values.
 * Returns no value for NaN or invalid configuration so callers can leave staged data unchanged.
 * Stateless; safe to call concurrently.
 *
 * \param value Physical signal value.
 * \param scalingFactor Finite, nonzero scale in physical = raw * scale + offset.
 * \param offset Finite physical offset.
 * \param bitLength Wire width, from 1 to 64 bits.
 * \return Converted raw value, or no value for NaN or invalid configuration.
 */
etl::optional<uint64_t>
physicalToRaw(double value, double scalingFactor, double offset, uint8_t bitLength);

} // namespace middleware::core
