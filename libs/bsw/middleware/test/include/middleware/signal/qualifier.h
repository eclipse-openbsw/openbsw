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

#include <gmock/gmock.h>

#include <etl/span.h>
#include <cstdint>

#include <middleware/core/IQualifier.h>

namespace middleware::signal::integration
{

class Qualifier : public middleware::core::IQualifier
{
public:
    enum class Qualification : uint8_t
    {
        Ok,
        Error
    };

    MOCK_METHOD(void, qualify, (uintptr_t const, etl::span<uint8_t const>), (override));
    MOCK_METHOD(Qualification, getQualification, (), (const));

    void ON_CALL_GET_QUALIFICATION(Qualification const qualification)
    {
        ON_CALL(*this, getQualification()).WillByDefault(::testing::Return(qualification));
    }
};

} // namespace middleware::signal::integration
