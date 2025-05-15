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
    MOCK_METHOD(ErrorCode, bind, (ip::IPAddress const*, uint16_t));
    MOCK_METHOD(ErrorCode, join, (ip::IPAddress const&));
    MOCK_METHOD(bool, isBound, (), (const));
    MOCK_METHOD(void, close, ());
    MOCK_METHOD(bool, isClosed, (), (const));
    MOCK_METHOD(ErrorCode, connect, (ip::IPAddress const&, uint16_t, ip::IPAddress*));
    MOCK_METHOD(void, disconnect, ());
    MOCK_METHOD(bool, isConnected, (), (const));
    MOCK_METHOD(size_t, read, (uint8_t*, size_t));
    MOCK_METHOD(ErrorCode, send, (::etl::span<uint8_t const> const&));
    MOCK_METHOD(ErrorCode, send, (DatagramPacket const&));
    MOCK_METHOD(ip::IPAddress const*, getIPAddress, (), (const));
    MOCK_METHOD(ip::IPAddress const*, getLocalIPAddress, (), (const));
    MOCK_METHOD(uint16_t, getPort, (), (const));
    MOCK_METHOD(uint16_t, getLocalPort, (), (const));

    IDataListener* getDataListener() { return _dataListener; }

    IDataSentListener* getDataSentListener() { return _dataSentListener; }
};

namespace test
{
using ReadBytesFrom = ::ethernet::test::ReadBytesFrom;
using WriteBytesTo  = ::ethernet::test::WriteBytesTo<AbstractDatagramSocket::ErrorCode>;
} // namespace test
} // namespace udp
