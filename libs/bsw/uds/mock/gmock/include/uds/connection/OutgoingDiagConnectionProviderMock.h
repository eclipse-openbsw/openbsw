/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    MOCK_METHOD(
        ErrorCode,
        getOutgoingDiagConnection,
        (uint8_t, OutgoingDiagConnection*&, transport::TransportMessage*),
        (override));
};

} // namespace uds
