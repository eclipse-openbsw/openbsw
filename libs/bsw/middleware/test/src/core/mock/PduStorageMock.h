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

#include <etl/span.h>

#include <gmock/gmock.h>

#include <cstdint>

namespace middleware::core::test
{

/** \brief Mocks the load and store functions of the PDUs, which are plain function pointers. */
class PduStorageMock
{
public:
    MOCK_METHOD(::etl::span<uint8_t const>, load, (uint32_t, ::etl::span<uint8_t>));
    MOCK_METHOD(bool, store, (uint32_t, ::etl::span<uint8_t const>));
};

} // namespace middleware::core::test
