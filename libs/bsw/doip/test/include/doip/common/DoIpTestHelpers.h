/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <async/TestContext.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <tcp/socket/AbstractSocketMock.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

inline bool is_equal(::etl::span<uint8_t const> const a, ::etl::span<uint8_t const> const b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    return (a.size() == 0) || (0 == ::etl::mem_compare(a.begin(), a.end(), b.begin()));
}

class DoIpExpectSendHelper
{
public:
    template<size_t N>
    DoIpExpectSendHelper(::tcp::AbstractSocketMock& socketMock, uint8_t const (&message)[N])
    : _socketMock(socketMock), _message(message), _size(N), _unsentSize(N)
    {}

    Sequence& getSequence() { return _seq; }

    DoIpExpectSendHelper& send(uint32_t size);
    DoIpExpectSendHelper& flush();
    DoIpExpectSendHelper& verify(::async::TestContext& testContext);
    void dataSentAndVerify(::async::TestContext& testContext);

private:
    Sequence _seq;
    ::tcp::AbstractSocketMock& _socketMock;
    uint8_t const* _message;
    uint32_t _size;
    uint32_t _unsentSize;
};

class DoIpExpectReadHelper
{
public:
    template<size_t N>
    DoIpExpectReadHelper(::tcp::AbstractSocketMock& socketMock, uint8_t const (&message)[N])
    : _socketMock(socketMock), _message(message), _size(N), _unreadSize(N)
    {}

    Sequence& getSequence() { return _seq; }

    DoIpExpectReadHelper& read(uint32_t size);
    DoIpExpectReadHelper& skip(uint32_t size);
    void dataReceivedAndVerify(::async::TestContext& testContext);

private:
    Sequence _seq;
    ::tcp::AbstractSocketMock& _socketMock;
    uint8_t const* _message;
    uint32_t _size;
    uint32_t _unreadSize;
};

} // namespace test
} // namespace doip
