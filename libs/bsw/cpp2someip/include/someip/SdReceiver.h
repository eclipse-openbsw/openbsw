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
#include "someip/ISdMessageParser.h"
#include "someip/SomeIpMessage.h"

#include <cstdint>

namespace someip
{
class SdReceiver : public INetworkListener
{
public:
    explicit SdReceiver(ISdMessageParser& parser);

    /** \see INetworkListener::received */
    void received(NetworkChannel& channel, uint32_t length) override;

private:
    static uint32_t const MAX_SD_PAYLOAD_LENGTH = 1400U; // UDP

    void handleMessage(NetworkChannel& channel, SomeIpMessage const& message);

    ISdMessageParser& _parser;
};

} // namespace someip
