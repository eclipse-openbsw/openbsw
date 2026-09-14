/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

namespace transport
{
class TransportMessage;
}

namespace uds
{
class OutgoingDiagConnection;

class IOutgoingDiagConnectionProvider
{
public:
    enum ErrorCode
    {
        CONNECTION_OK,
        NO_CONNECTION_AVAILABLE,
        GENERAL_ERROR
    };

    virtual ErrorCode getOutgoingDiagConnection(
        uint16_t targetAddress,
        OutgoingDiagConnection*& pConnection,
        transport::TransportMessage* pRequestMessage)
        = 0;
};

} // namespace uds
