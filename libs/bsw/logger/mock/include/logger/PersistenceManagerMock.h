/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <logger/IPersistenceManager.h>

#include <gmock/gmock.h>

namespace logger
{
class PersistenceManagerMock : public IPersistenceManager
{
public:
    MOCK_CONST_METHOD1(writeMapping, bool(::etl::span<uint8_t const> const&));
    MOCK_CONST_METHOD1(readMapping, ::etl::span<uint8_t const>(::etl::span<uint8_t>));
};

} // namespace logger
