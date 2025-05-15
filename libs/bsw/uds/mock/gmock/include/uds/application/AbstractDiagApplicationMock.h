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

    MOCK_METHOD(
        void,
        responseReceived,
        (OutgoingDiagConnection&, uint8_t, uint8_t const[], uint16_t),
        (override));

    MOCK_METHOD(void, responseTimeout, (OutgoingDiagConnection&), (override));

    MOCK_METHOD(
        void,
        requestSent,
        (OutgoingDiagConnection & connection, RequestSendResult result),
        (override));
};

} // namespace uds
