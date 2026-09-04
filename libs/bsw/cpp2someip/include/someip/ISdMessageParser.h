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

#include "someip/SomeIpMessage.h"

#include <ip/IPEndpoint.h>

#include <etl/delegate.h>
#include <etl/optional.h>

namespace someip
{
class ISdMessageParser
{
protected:
    ISdMessageParser() = default;

public:
    using AdditionalSDCheck = ::etl::optional<::etl::delegate<bool(::ip::IPEndpoint const&)>>;

    ISdMessageParser(ISdMessageParser const&)            = delete;
    ISdMessageParser& operator=(ISdMessageParser const&) = delete;

    virtual ~ISdMessageParser() = default;

    /**
     * Pure virtual function that handles SomeIpMessages.
     */
    virtual void handleMessage(
        SomeIpMessage const& message, ::ip::IPEndpoint const& sourceEndpoint, bool isMulticast)
        = 0;
};
} // namespace someip
