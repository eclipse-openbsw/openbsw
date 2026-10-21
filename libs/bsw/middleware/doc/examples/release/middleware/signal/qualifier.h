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

#include <cstddef>
#include <cstdint>

#include <etl/span.h>

#include <middleware/core/IQualifier.h>

namespace middleware::signal::integration
{

/**
 * \brief Minimal qualifier used in the examples.
 *
 * \note The application owns the qualifier and injects it into each reader; it is not
 * thread-safe.
 */
class Qualifier final : public middleware::core::IQualifier
{
public:
    enum class Qualification : uint8_t
    {
        Ok,
        Error
    };

    /**
     * \brief Marks a stable example PDU snapshot as qualified.
     */
    void
    qualify(uintptr_t const /* lastSeqNumber */, etl::span<uint8_t const> /* rawData */) override
    {
        state_ = Qualification::Ok;
    }

    /**
     * \brief Returns the status of the latest qualification.
     */
    [[nodiscard]] Qualification getQualification() const { return state_; }

private:
    Qualification state_{Qualification::Error};
};

} // namespace middleware::signal::integration
