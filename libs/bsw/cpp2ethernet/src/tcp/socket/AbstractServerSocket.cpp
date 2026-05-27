/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
