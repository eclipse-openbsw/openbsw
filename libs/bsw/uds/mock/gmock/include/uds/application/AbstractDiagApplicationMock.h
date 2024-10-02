// Copyright 2024 Accenture.

#ifndef GUARD_D3231C69_989E_42DF_9DD6_1185C17A5799
#define GUARD_D3231C69_989E_42DF_9DD6_1185C17A5799

#include "application/AbstractDiagApplication.h"

#include <mock/gmock.h>

namespace uds
{
class AbstractDiagApplicationMock : public AbstractDiagApplication
{
public:
    AbstractDiagApplicationMock(IOutgoingDiagConnectionProvider& connectionProvider)
    : AbstractDiagApplication(connectionProvider)
    {}

    MOCK_METHOD4(
        responseReceived, void(OutgoingDiagConnection&, uint8_t, uint8_t const[], uint16_t));

    MOCK_METHOD1(responseTimeout, void(OutgoingDiagConnection&));

    MOCK_METHOD2(requestSent, void(OutgoingDiagConnection& connection, RequestSendResult result));
};

} // namespace uds

#endif // GUARD_D3231C69_989E_42DF_9DD6_1185C17A5799
