// Copyright 2024 Accenture.

#pragma once

#include <util/stream/IOutputStream.h>

#include <gmock/gmock.h>

namespace util
{
namespace stream
{
class OutputStreamMock : public IOutputStream
{
public:
    MOCK_METHOD(bool, isEof, (), (const, override));
    MOCK_METHOD(void, write, (uint8_t data), (override));
    MOCK_METHOD(void, write, (::estd::slice<uint8_t const> const& buffer), (override));
};

} // namespace stream
} // namespace util
