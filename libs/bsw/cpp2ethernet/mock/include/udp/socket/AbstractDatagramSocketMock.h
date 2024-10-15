// Copyright 2025 Accenture.

#pragma once

#include "cpp2ethernet/gtest_extensions.h"
#include "udp/DatagramPacket.h"
#include "udp/socket/AbstractDatagramSocket.h"

#include <gmock/gmock.h>

namespace udp
{
struct AbstractDatagramSocketMock : public AbstractDatagramSocket
{
    MOCK_METHOD2(bind, ErrorCode(ip::IPAddress const*, uint16_t));
    MOCK_METHOD1(join, ErrorCode(ip::IPAddress const&));
    MOCK_CONST_METHOD0(isBound, bool());
    MOCK_METHOD0(close, void());
    MOCK_CONST_METHOD0(isClosed, bool());
    MOCK_METHOD3(connect, ErrorCode(ip::IPAddress const&, uint16_t, ip::IPAddress*));
    MOCK_METHOD0(disconnect, void());
    MOCK_CONST_METHOD0(isConnected, bool());
    MOCK_METHOD2(read, size_t(uint8_t*, size_t));
    MOCK_METHOD1(send, ErrorCode(::etl::span<uint8_t const> const&));
    MOCK_METHOD1(send, ErrorCode(DatagramPacket const&));
    MOCK_CONST_METHOD0(getIPAddress, ip::IPAddress const*());
    MOCK_CONST_METHOD0(getLocalIPAddress, ip::IPAddress const*());
    MOCK_CONST_METHOD0(getPort, uint16_t());
    MOCK_CONST_METHOD0(getLocalPort, uint16_t());

    IDataListener* getDataListener() { return _dataListener; }

    IDataSentListener* getDataSentListener() { return _dataSentListener; }
};

namespace test
{
using ReadBytesFrom = ::ethernet::test::ReadBytesFrom;
using WriteBytesTo  = ::ethernet::test::WriteBytesTo<AbstractDatagramSocket::ErrorCode>;
} // namespace test
} // namespace udp
