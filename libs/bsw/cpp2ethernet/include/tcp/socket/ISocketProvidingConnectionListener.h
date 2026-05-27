/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "ip/IPAddress.h"
#include "tcp/socket/AbstractSocket.h"

#include <platform/estdint.h>

namespace tcp
{
/**
 * Interface for classes that provide Sockets
 */
class ISocketProvidingConnectionListener
{
public:
    virtual ~ISocketProvidingConnectionListener()                                 = default;
    ISocketProvidingConnectionListener()                                          = default;
    ISocketProvidingConnectionListener(ISocketProvidingConnectionListener const&) = delete;
    ISocketProvidingConnectionListener& operator=(ISocketProvidingConnectionListener const&)
        = delete;

    // [ISocketProvidingConnectionListener]
    /**
     * returns a pointer to a AbstractSocket object
     * \param ipAddr Ip address of client which tries to connect.
     * \param port Port of client from which it tries to connect.
     * \return
     *          - pointer to AbstractSocket
     *          - NULL if no more sockets are available
     */
    virtual AbstractSocket* getSocket(ip::IPAddress const& ipAddr, uint16_t port) = 0;

    /**
     * callback being called when a TCP connection gets accepted
     * \param  socket  the AbstractSocket representing the connection
     *
     * Usually the ISocketConnectionListener sets an IDataListener to the
     * socket when the connection is established.
     */
    virtual void connectionAccepted(AbstractSocket& socket) = 0;
    // [ISocketProvidingConnectionListener]
};

} /*namespace tcp*/
