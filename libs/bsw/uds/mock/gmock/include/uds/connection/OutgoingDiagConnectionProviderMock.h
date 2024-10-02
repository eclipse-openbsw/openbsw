// Copyright 2024 Accenture.

#ifndef GUARD_24D0D23D_4EDA_474F_8010_6382121C9295
#define GUARD_24D0D23D_4EDA_474F_8010_6382121C9295

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

#endif // GUARD_24D0D23D_4EDA_474F_8010_6382121C9295
