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

#include "doip/server/IDoIpServerTransportConnectionPool.h"

#include <gmock/gmock.h>

namespace doip
{
class DoIpServerTransportConnection;
class DoIpServerTransportConnectionConfig;

/**
 * Interface for a DoIP transport connection pool.
 */
class DoIpServerTransportConnectionPoolMock : public IDoIpServerTransportConnectionPool
{
public:
    MOCK_METHOD4(
        createConnection,
        DoIpServerTransportConnection*(
            uint8_t socketGroupId,
            ::tcp::AbstractSocket& socket,
            DoIpServerTransportConnectionConfig const& config,
            DoIpTcpConnection::ConnectionType type));
    MOCK_METHOD1(releaseConnection, void(DoIpServerTransportConnection& connection));
};

} // namespace doip
