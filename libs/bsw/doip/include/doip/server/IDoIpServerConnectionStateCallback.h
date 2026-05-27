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

namespace doip
{
/**
 * Callback interface to be notified about opened and closed DoIP connections.
 */
class IDoIpServerConnectionStateCallback
{
public:
    /**
     * Called if a DoIP connection is opened.
     * \param sourceAddress source address of the external tester
     */
    virtual void
    connectionRoutingActive(uint16_t sourceAddress, DoIpTcpConnection::ConnectionType type)
        = 0;
    /**
     * Called if a DoIP connection is closed.
     * \param sourceAddress source address of the external tester
     */
    virtual void connectionClosed(uint16_t sourceAddress) = 0;
};

} // namespace doip
