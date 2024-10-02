// Copyright 2024 Accenture.

#ifndef GUARD_71187552_1C3B_4FD3_9663_3DCC02B2C88D
#define GUARD_71187552_1C3B_4FD3_9663_3DCC02B2C88D

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
        transport::TransportMessage* pRequestMessage = nullptr)
        = 0;
};

} // namespace uds

#endif // GUARD_71187552_1C3B_4FD3_9663_3DCC02B2C88D
