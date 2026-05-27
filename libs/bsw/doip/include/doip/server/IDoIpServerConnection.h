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

namespace doip
{
/**
 * Interface for a DoIP server connection. This is an extended DoIP connection
 * that will be given to server message handlers.
 */
class IDoIpServerConnection : public IDoIpConnection
{
public:
    /**
     * Get the source address for this connection.
     * \return source address
     */
    virtual uint16_t getSourceAddress() const = 0;

    /**
     * Get the internal source address for this connection (address used for routing).
     * \return internal source address
     */
    virtual uint16_t getInternalSourceAddress() const = 0;

    /**
     * Send a NACK message on the connection.
     * \param nackCode NACK code as defined in ISO 13400
     * \param closeAfterSend true if the connection should close after sending has been done
     */
    virtual void sendNack(uint8_t nackCode, bool closeAfterSend) = 0;

protected:
    ~IDoIpServerConnection() = default;
};

} // namespace doip
