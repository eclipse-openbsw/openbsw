// Copyright 2025 Accenture.

#include "tcp/util/TcpIperf2Server.h"

#include "tcp/TcpLogger.h"
#include "tcp/socket/AbstractSocket.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace tcp
{
using ::util::logger::Logger;
using ::util::logger::TCP;

TcpIperf2Server::TcpIperf2Server(AbstractSocket& socket) : _locked(false), _socket(socket) {}

AbstractSocket* TcpIperf2Server::getSocket(ip::IPAddress const&, uint16_t)
{
    if (!_locked)
    {
        _locked = true;
        return &_socket;
    }

    return nullptr;
}

void TcpIperf2Server::connectionAccepted(AbstractSocket& socket) { socket.setDataListener(this); }

void TcpIperf2Server::dataReceived(uint16_t const length)
{
    // Discard Data
    (void)_socket.read(nullptr, length);
    Logger::info(TCP, "Length of dataReceived:  %d", length);
}

void TcpIperf2Server::connectionClosed(ErrorCode /* status */) { _locked = false; }

} // namespace tcp

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
