/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/server/IDoIpServerConnectionFilter.h"

namespace doip
{
class DoIpServerConnectionHandler;

/**
 * Callback interface for DoIP connection handler.
 */
class IDoIpServerConnectionHandlerCallback : public IDoIpServerConnectionFilter
{
public:
    /**
     * Called if a valid routing activation request has been received and should now be
     * handled. This method should be used to check for conflicts with other connections.
     * \param handler reference to the connection handler. Here routingActivationCompleted()
     *                should be called when a result is available
     */
    virtual void handleRoutingActivationRequest(DoIpServerConnectionHandler& handler) = 0;

    /**
     * Called if routing is active on a given connection.
     * \param handler reference to the connection handler
     */
    virtual void routingActive(DoIpServerConnectionHandler& handler) = 0;

    /**
     * Called if an alive check response has been received after the method startAliveCheck()
     * has been called.
     * \param handler reference to the connection handler
     * \param isAlive true if the connection is alive i.e. a valid alive check response has been
     *        received in time
     */
    virtual void aliveCheckResponseReceived(DoIpServerConnectionHandler& handler, bool isAlive) = 0;

    /**
     * Called to indicate that a connection has closed.
     * \param handler reference to the connection handler
     */
    virtual void connectionClosed(DoIpServerConnectionHandler& handler) = 0;

protected:
    ~IDoIpServerConnectionHandlerCallback() = default;
};

} // namespace doip
