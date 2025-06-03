// Copyright 2024 Accenture.

#pragma once

#include <logger/IPersistenceManager.h>

#include <gmock/gmock.h>

namespace logger
{
class PersistenceManagerMock : public IPersistenceManager
{
public:
    MOCK_METHOD(bool, writeMapping, (::estd::slice<uint8_t const> const&), (const, override));
    MOCK_METHOD(
        ::estd::slice<uint8_t const>, readMapping, (::estd::slice<uint8_t>), (const, override));
};

} // namespace logger
