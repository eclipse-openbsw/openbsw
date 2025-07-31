// Copyright 2025 Accenture.

#include "udp/socket/AbstractDatagramSocket.h"

namespace udp
{
AbstractDatagramSocket::AbstractDatagramSocket()
: _dataListener(nullptr), _dataSentListener(nullptr)
{}

} // namespace udp
