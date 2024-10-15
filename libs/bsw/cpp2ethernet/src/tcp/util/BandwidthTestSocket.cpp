// Copyright 2025 Accenture.

#include "tcp/util/BandwidthTestSocket.h"

#include "tcp/socket/AbstractSocket.h"

namespace tcp
{
BandwidthTestSocket::BandwidthTestSocket(AbstractSocket& socket) : _locked(false), _socket(socket)
{}

AbstractSocket*
BandwidthTestSocket::getSocket(ip::IPAddress const& /* ipAddr */, uint16_t /* port */)
{
    if (!_locked)
    {
        _locked = true;
        return &_socket;
    }

    return nullptr;
}

void BandwidthTestSocket::connectionAccepted(AbstractSocket& socket)
{
    socket.setDataListener(this);
}

void BandwidthTestSocket::dataReceived(uint16_t length) { (void)_socket.read(nullptr, length); }

void BandwidthTestSocket::connectionClosed(ErrorCode) { _locked = false; }

} // namespace tcp
