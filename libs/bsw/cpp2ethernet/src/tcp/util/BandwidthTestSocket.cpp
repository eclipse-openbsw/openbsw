/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
