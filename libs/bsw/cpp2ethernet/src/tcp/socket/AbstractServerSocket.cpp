// Copyright 2025 Accenture.

#include "tcp/socket/AbstractServerSocket.h"

namespace tcp
{
AbstractServerSocket::AbstractServerSocket()
: _port(0U), _socketProvidingConnectionListener(nullptr)
{}

AbstractServerSocket::AbstractServerSocket(
    uint16_t const port, ISocketProvidingConnectionListener& providingListener)
: _port(port), _socketProvidingConnectionListener(&providingListener)
{}

} // namespace tcp
