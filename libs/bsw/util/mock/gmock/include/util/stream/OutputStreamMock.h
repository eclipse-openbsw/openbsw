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
    MOCK_CONST_METHOD0(isEof, bool());
    MOCK_METHOD1(write, void(uint8_t data));
    MOCK_METHOD1(write, void(::estd::slice<uint8_t const> const& buffer));
};

} // namespace stream
} // namespace util

