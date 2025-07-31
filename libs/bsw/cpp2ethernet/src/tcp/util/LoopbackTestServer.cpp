// Copyright 2025 Accenture.

#include "tcp/util/LoopbackTestServer.h"

#include "tcp/TcpLogger.h"
#include "tcp/socket/AbstractSocket.h"

#include <util/estd/memory.h>

namespace tcp
{
namespace memory = ::estd::memory;

using ::util::logger::Logger;
using ::util::logger::TCP;

LoopbackTestServer::LoopbackTestServer(AbstractSocket& socket) : _locked(false), _socket(socket) {}

AbstractSocket* LoopbackTestServer::getSocket(ip::IPAddress const&, uint16_t)
{
    if (!_locked)
    {
        _locked = true;
        return &_socket;
    }

    return nullptr;
}

void LoopbackTestServer::connectionAccepted(AbstractSocket& socket)
{
    socket.setDataListener(this);
}

void LoopbackTestServer::sendByte(uint8_t byte)
{
    AbstractSocket::ErrorCode result;
    result = _socket.send(::estd::memory::as_span(&byte));

    switch (result)
    {
        case AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_ERR_NOT_OK");
            break;
        }
        case AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OPEN:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_ERR_NOT_OPEN");
            break;
        }
        case AbstractSocket::ErrorCode::SOCKET_ERR_NO_MORE_BUFFER:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_ERR_NO_MORE_BUFFER");
            break;
        }
        case AbstractSocket::ErrorCode::SOCKET_FLUSH:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_FLUSH");
            break;
        }
        default: // the SOCKET_ERR_OK case
            break;
    }
}

void LoopbackTestServer::sendData(::etl::span<uint8_t> data)
{
    AbstractSocket::ErrorCode result;
    result = _socket.send(data);

    switch (result)
    {
        case AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OK:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_ERR_NOT_OK");
            break;
        }
        case AbstractSocket::ErrorCode::SOCKET_ERR_NOT_OPEN:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_ERR_NOT_OPEN");
            break;
        }
        case AbstractSocket::ErrorCode::SOCKET_ERR_NO_MORE_BUFFER:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_ERR_NO_MORE_BUFFER");
            break;
        }
        case AbstractSocket::ErrorCode::SOCKET_FLUSH:
        {
            Logger::error(TCP, "Loopback: send() returned SOCKET_FLUSH");
            break;
        }
        default: // the SOCKET_ERR_OK case
            break;
    }
}

void LoopbackTestServer::dataReceived(uint16_t const length)
{
    if (length > sizeof(_receiveData))
    {
        // Discard Data
        (void)_socket.read(nullptr, length);
        Logger::error(TCP, "Received oversized packet.");
    }
    else
    {
        (void)_socket.read(&_receiveData[0U], length);
        Logger::info(TCP, "Length of dataReceived:  %d", length);
        sendData(::etl::span<uint8_t>(_receiveData.data(), length));
    }
}

void LoopbackTestServer::connectionClosed(ErrorCode /* status */) { _locked = false; }

} // namespace tcp
