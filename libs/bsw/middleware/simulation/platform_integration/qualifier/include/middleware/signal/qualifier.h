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

#include <etl/crc8_ccitt.h>
#include <etl/span.h>
#include <cstdint>

#include <middleware/core/IQualifier.h>

namespace middleware::signal::integration
{

class Qualifier final : public middleware::core::IQualifier
{
public:
    enum class Qualification : uint8_t
    {
        Ok,
        Error
    };

    /**
     * \brief Checks the PDU CRC stored in the first byte against the remaining payload.
     */
    void qualify(uintptr_t const /* seqNumber */, etl::span<uint8_t const> data) override
    {
        if (data.empty())
        {
            state_ = Qualification::Error;
        }
        else
        {
            uint8_t const calculatedCrc = etl::crc8_ccitt(data.begin() + 1U, data.end());
            state_ = (calculatedCrc == data.at(0)) ? Qualification::Ok : Qualification::Error;
        }
    }

    /**
     * \brief Returns whether the latest PDU passed qualification.
     */
    [[nodiscard]] Qualification getQualification() const { return state_; }

private:
    Qualification state_{Qualification::Error};
};

} // namespace middleware::signal::integration
