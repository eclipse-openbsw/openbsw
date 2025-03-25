// Copyright 2024 Accenture.

#pragma once

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
