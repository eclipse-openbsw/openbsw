// Copyright 2025 Accenture.

#pragma once

#include "cpp2ethernet/gtest_extensions.h"
#include "tcp/IDataListener.h"
#include "tcp/IDataSendNotificationListener.h"
#include "tcp/socket/AbstractSocket.h"

#include <etl/span.h>
#include <util/estd/memory.h>

#include <gmock/gmock.h>

#include <cstring>

namespace tcp
{
using namespace ::testing;

struct AbstractSocketMock : public AbstractSocket
{
    virtual ~AbstractSocketMock() = default;

    AbstractSocketMock()
    : AbstractSocket()
    , _injectedData{}
    , _dataWriteWindow{_injectedData}
    , _dataReadWindow{_injectedData}
    {
        _dataReadWindow = {};
    }

    MOCK_METHOD2(bind, ErrorCode(ip::IPAddress const&, uint16_t const));
    MOCK_METHOD3(connect, ErrorCode(ip::IPAddress const&, uint16_t const, ConnectedDelegate));
    MOCK_METHOD0(close, ErrorCode());
    MOCK_METHOD0(abort, void());
    MOCK_METHOD0(flush, ErrorCode());
    MOCK_METHOD0(discardData, void());
    MOCK_METHOD0(available, size_t());
    MOCK_METHOD1(read, uint8_t(uint8_t&));
    MOCK_METHOD2(read, size_t(uint8_t*, size_t));
    MOCK_METHOD1(send, ErrorCode(::etl::span<uint8_t const> const&));
    MOCK_CONST_METHOD0(isClosed, bool());
    MOCK_CONST_METHOD0(isEstablished, bool());
    MOCK_CONST_METHOD0(getRemoteIPAddress, ip::IPAddress());
    MOCK_CONST_METHOD0(getLocalIPAddress, ip::IPAddress());
    MOCK_CONST_METHOD0(getRemotePort, uint16_t());
    MOCK_CONST_METHOD0(getLocalPort, uint16_t());
    MOCK_METHOD0(disableNagleAlgorithm, void());
    MOCK_METHOD3(enableKeepAlive, void(uint32_t, uint32_t, uint32_t));
    MOCK_METHOD0(disableKeepAlive, void());

    void signalReceivedData(size_t length)
    {
        if (getDataListener() != nullptr)
        {
            getDataListener()->dataReceived(length);
        }
    }

    void signalClosed(IDataListener::ErrorCode status)
    {
        if (getDataListener() != nullptr)
        {
            getDataListener()->connectionClosed(status);
        }
    }

    void signalDataSent(size_t length)
    {
        if (getSendNotificationListener() != nullptr)
        {
            getSendNotificationListener()->dataSent(
                length, IDataSendNotificationListener::SendResult::DATA_SENT);
        }
    }

    ErrorCode inject(::etl::span<uint8_t const> const data)
    {
        // safeguard in case a test tries to inject more data than _injectedData can hold
        estd_assert(_dataWriteWindow.size() >= data.size());

        ::estd::memory::copy(_dataWriteWindow, data);
        _dataWriteWindow.advance(data.size());

        _dataReadWindow = ::etl::span<uint8_t const>(
            _dataReadWindow.data(), _dataReadWindow.size() + data.size());

        signalReceivedData(data.size());
        return ErrorCode::SOCKET_ERR_OK;
    }

    size_t readImplementation(uint8_t* const buffer, size_t const length)
    {
        if (length > _dataReadWindow.size())
        {
            return 0U;
        }

        if (buffer != nullptr)
        {
            auto const destination = ::etl::span<uint8_t>(buffer, length);
            ::estd::memory::copy(destination, _dataReadWindow.subspan(length));
        }
        _dataReadWindow.advance(length);

        if (_dataReadWindow.size() == 0)
        {
            // All injected data was read.
            // Reset write position to beginning of receive buffer.
            _dataWriteWindow = _injectedData;
            _dataReadWindow  = _injectedData;
            _dataReadWindow  = _dataReadWindow.subspan(0);
        }

        return length;
    }

    ErrorCode sendImplementation(::etl::span<uint8_t const> const& data)
    {
        printf("sendImplementation(%zu)\n", data.size());
        return AbstractSocket::ErrorCode::SOCKET_ERR_OK;
    }

    static void
    connectSocketMocks(tcp::AbstractSocketMock& clientSocket, tcp::AbstractSocketMock& serverSocket)
    {
        // client -> server
        ON_CALL(clientSocket, send(_))
            .WillByDefault(Invoke(&serverSocket, &tcp::AbstractSocketMock::inject));
        ON_CALL(serverSocket, read(_, _))
            .WillByDefault(Invoke(&serverSocket, &tcp::AbstractSocketMock::readImplementation));

        // server -> client
        ON_CALL(serverSocket, send(_))
            .WillByDefault(Invoke(&clientSocket, &tcp::AbstractSocketMock::inject));
        ON_CALL(clientSocket, read(_, _))
            .WillByDefault(Invoke(&clientSocket, &tcp::AbstractSocketMock::readImplementation));
    }

private:
    ::etl::array<uint8_t, 1024 * 16> _injectedData{};
    ::etl::span<uint8_t> _dataWriteWindow;
    ::etl::span<uint8_t const> _dataReadWindow;
};

namespace test
{
using ReadBytesFrom = ::ethernet::test::ReadBytesFrom;
using WriteBytesTo  = ::ethernet::test::WriteBytesTo<AbstractSocket::ErrorCode>;

} // namespace test
} // namespace tcp
