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

#include "doip/common/IDoIpConnection.h"

#include <etl/variant.h>

namespace doip
{
struct DoIpHeader;

/**
 * Callback interface for a DoIP connection.
 */
class IDoIpConnectionHandler
{
public:
    /**
     * Tag type to indicate that the message is handled by this handler.
     */
    struct HandledByThisHandler
    {
        bool operator==(HandledByThisHandler const&) const { return true; }
    };

    /**
     * Variant type to indicate whether the message is handled by this handler. If not, optionally a
     * callback upon payload discard can be specified.
     */
    using HeaderReceivedContinuation
        = ::etl::variant<HandledByThisHandler, IDoIpConnection::PayloadDiscardedCallbackType>;

    IDoIpConnectionHandler() {}

    /**
     * Called when the underlying connection is being closed.
     * \param closedByRemotePeer flag indicating whether the connection has been closed
     *        by the remote peer. If false the local socket has somehow be closed
     */
    virtual void connectionClosed(bool closedByRemotePeer) = 0;

    /**
     * Called to indicate that at least a header of a DoIP message has already
     * been received. Use IDoIpConnection::receivePayload() to receive the message payload
     * or arbitrary parts of it and IDoIpConnection::endReceiveMessage() to ignore the
     * rest of the payload.
     * \note there may be restrictions for certain implementations of IDoIpConnection such
     *       as that payload has to be received synchronously from within headerReceived()
     * \param header reference to the header
     * \return a HeaderReceivedContinuation object indicating whether the message is handled by this
     * handler
     */

    virtual HeaderReceivedContinuation headerReceived(DoIpHeader const& header) = 0;

    virtual ~IDoIpConnectionHandler()                                = default;
    IDoIpConnectionHandler& operator=(IDoIpConnectionHandler const&) = delete;
    IDoIpConnectionHandler(IDoIpConnectionHandler const&)            = delete;
};

} // namespace doip
