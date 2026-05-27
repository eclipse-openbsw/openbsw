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

#include <etl/intrusive_links.h>

namespace doip
{
struct DoIpHeader;
class IDoIpServerConnection;

/**
 * Interface for a DoIP message handler. A DoIP message handler can receive
 * and send DoIP messages over a already managed connection.
 */
class IDoIpServerMessageHandler : public ::etl::bidirectional_link<0>
{
public:
    /**
     * Called when a connection has been opened but no routing activation
     * has been performed yet.
     * \param connection reference to the connection that from now on can be
     *        used by the message handler
     */
    virtual void connectionOpened(IDoIpServerConnection& connection) = 0;

    /**
     * Called when routing activation has succeeded.
     */
    virtual void routingActive() = 0;

    /**
     * Called when a header (not handled by the server connection handler) has
     * been received.
     * \param header reference to the header
     * \return true if the message handler is responsible for the received message. It
     *         is then responsible for calling endReceiveMessage() after processing.
     */
    virtual bool headerReceived(DoIpHeader const& header) = 0;

    /**
     * Called when the connection has been closed.
     */
    virtual void connectionClosed() = 0;

protected:
    ~IDoIpServerMessageHandler() = default;
};

} // namespace doip
