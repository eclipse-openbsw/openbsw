// Copyright 2024 Accenture.

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
