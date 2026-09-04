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

#include "someip/ITpListener.h"
#include "someip/ITpTransceiver.h"

#include <gmock/gmock.h>

namespace someip
{
class TpTransceiverMock : public ITpTransceiver
{
public:
    MOCK_METHOD(
        bool, sendTpMessage, (NetworkChannel & channel, SomeIpMessage const& message), (const));

    MOCK_METHOD(
        void,
        receiveTpMessage,
        (NetworkChannel & channel, SomeIpMessage const& message, ITpListener& listener));
};

} // namespace someip
