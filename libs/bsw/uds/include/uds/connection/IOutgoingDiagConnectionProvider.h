// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

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
        uint16_t targetId,
        OutgoingDiagConnection*& pConnection,
        transport::TransportMessage* pRequestMessage)
        = 0;
};

} // namespace uds
