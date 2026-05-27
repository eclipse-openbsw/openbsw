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

#include "doip/server/IDoIpServerMessageHandler.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Interface for a DoIp server message handler.
 */
class DoIpServerMessageHandlerMock : public IDoIpServerMessageHandler
{
public:
    MOCK_METHOD1(connectionOpened, void(IDoIpServerConnection& connection));
    MOCK_METHOD0(routingActive, void());
    MOCK_METHOD1(headerReceived, bool(DoIpHeader const& header));
    MOCK_METHOD0(connectionClosed, void());
};

} // namespace doip
