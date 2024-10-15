// Copyright 2025 Accenture.

#pragma once

#include <etl/span.h>
#include <util/estd/memory.h>

namespace ethernet
{
namespace test
{
/* Helper struct to mock read(uint8_t*, size_t) method of AbstractSocket, AbstractDatagramSocket
 *
 * Example:
 *   const uint8_t frame[10] = {};
 *   EXPECT_CALL(socketMock, read(_, _)).WillOnce(Invoke(ReadBytesFrom(frame)));
 *
 */
struct ReadBytesFrom
{
    ReadBytesFrom(::etl::span<uint8_t const> data) : src(data) {}

    size_t operator()(uint8_t* dest, size_t size) const
    {
        return ::estd::memory::copy(::etl::span<uint8_t>(dest, size), src).size();
    }

    ::etl::span<uint8_t const> src;
};

/* Helper struct to mock send(span<const uint8>) method of AbstractSocket, AbstractDatagramSocket
 *
 * Example:
 *   uint8_t frame[10] = {}
 *   EXPECT_CALL(socketMock, send(_)).WillOnce(Invoke(WriteBytesTo(frame)));
 *   EXPECT_THAT(frame)
 */

template<typename ErrorCode>
struct WriteBytesTo
{
    WriteBytesTo(::etl::span<uint8_t> dest, ErrorCode ec) : dest(dest), ec(ec) {}

    ErrorCode operator()(::etl::span<uint8_t const> src)
    {
        ::estd::memory::copy(dest, src);
        return ec;
    }

    ::etl::span<uint8_t> dest;
    ErrorCode ec;
};

} // namespace test
} // namespace ethernet
