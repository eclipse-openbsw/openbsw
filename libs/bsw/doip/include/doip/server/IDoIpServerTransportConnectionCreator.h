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

#include "doip/server/DoIpServerTransportConnection.h"

#include <etl/ipool.h>
#include <platform/estdint.h>

namespace tcp
{
class AbstractSocket;
}

namespace doip
{
class DoIpServerTransportConnectionConfig;

/**
 * Interface for creation of a potentially derived transport connection.
 * \tparam T class for transport connection instances, derived from DoIpServerTransportConnection
 */
template<class T>
class IDoIpServerTransportConnectionCreator
{
public:
    /**
     * Create a transport connection instance within already allocated memory.
     * \param memory reference to allocated memory
     * \param socketGroupId identifier of socket group the connection belongs to
     * \param socket reference to socket to use
     * \param diagnosticSendJobBlockPool reference to block pool for diagnostic send jobs
     * \param protocolSendJobBlockPool reference to block pool for other protocol send jobs
     * \param config reference to connection configuration
     */
    virtual DoIpServerTransportConnection& createConnection(
        T* memory,
        uint8_t socketGroupId,
        ::tcp::AbstractSocket& socket,
        ::etl::ipool& diagnosticSendJobBlockPool,
        ::etl::ipool& protocolSendJobBlockPool,
        DoIpServerTransportConnectionConfig const& config,
        DoIpTcpConnection::ConnectionType type)
        = 0;

protected:
    ~IDoIpServerTransportConnectionCreator() = default;
    IDoIpServerTransportConnectionCreator& operator=(IDoIpServerTransportConnectionCreator const&)
        = default;
};

} // namespace doip
