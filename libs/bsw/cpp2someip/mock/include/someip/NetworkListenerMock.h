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

#include "someip/INetworkListener.h"
#include "someip/NetworkChannel.h"

#include <gmock/gmock.h>

namespace someip
{
class NetworkListenerMock : public INetworkListener
{
public:
    MOCK_METHOD(void, received, (NetworkChannel & channel, uint32_t length));
};

} // namespace someip
