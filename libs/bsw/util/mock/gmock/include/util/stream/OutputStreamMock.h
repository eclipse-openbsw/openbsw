// Copyright 2024 Accenture.

#pragma once

#include <etl/span.h>
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
    MOCK_METHOD(void, write, (uint8_t data));
    MOCK_METHOD(void, write, (::etl::span<uint8_t const> const& buffer));
};

} // namespace stream
} // namespace util
