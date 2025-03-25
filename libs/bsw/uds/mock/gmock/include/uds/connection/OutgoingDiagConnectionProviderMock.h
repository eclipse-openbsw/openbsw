// Copyright 2024 Accenture.

#pragma once

#include "uds/connection/IOutgoingDiagConnectionProvider.h"

#include <gmock/gmock.h>

namespace transport
{
class TransportMessage;
}

namespace uds
{
class OutgoingDiagConnection;
}

namespace uds
{
class OutgoingDiagConnectionProviderMock : public IOutgoingDiagConnectionProvider
{
public:
    MOCK_METHOD3(
        getOutgoingDiagConnection,
        ErrorCode(uint8_t, OutgoingDiagConnection*&, transport::TransportMessage*));
};

} // namespace uds

