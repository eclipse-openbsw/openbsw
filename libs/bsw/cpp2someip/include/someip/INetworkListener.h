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

#include "someip/NetworkChannel.h"

#include <cstdint>

namespace someip
{
/**
 * Listener for incoming data via network channels.
 */
class INetworkListener
{
public:
    virtual ~INetworkListener() = default;

    /**
     * Signals data being received.
     *
     * \param channel which is receiving the data
     * \param length of data being received
     */
    virtual void received(NetworkChannel& channel, uint32_t length) = 0;
};

} // namespace someip
