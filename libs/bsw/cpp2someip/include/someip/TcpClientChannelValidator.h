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

#include "someip/INetwork.h"

#include <ip/IPEndpoint.h>

namespace someip
{
class TcpClientChannelValidator
{
public:
    class CachedValidator
    {
    public:
        explicit CachedValidator(TcpClientChannelValidator& validator);

        bool isChannelEstablished(::ip::IPEndpoint const& remote, uint16_t localPort);
        void checkClientChannel(::ip::IPEndpoint const& remote, uint16_t localPort);

    private:
        enum class CachedResult : uint16_t
        {
            NO_RESULT,
            POSITIVE_RESULT,
            NEGATIVE_RESULT,
            RESULT_PENDING
        };

        bool hasResult(::ip::IPEndpoint const& remote, uint16_t localPort);

        TcpClientChannelValidator& _validator;
        ::ip::IPEndpoint _remote;
        uint16_t _localPort;
        CachedResult _cachedResult;
    };

    TcpClientChannelValidator(INetwork& network, bool magicCookieEnabled);

    bool isChannelEstablished(::ip::IPEndpoint const& remote, uint16_t localPort) const;
    void checkClientChannel(::ip::IPEndpoint const& remote, uint16_t localPort) const;

private:
    INetwork& _network;
    bool _magicCookieEnabled;
};
} // namespace someip
