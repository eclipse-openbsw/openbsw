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

namespace someip
{
class IDiagnosticListener
{
protected:
    IDiagnosticListener() = default;

public:
    IDiagnosticListener(IDiagnosticListener const&)            = delete;
    IDiagnosticListener& operator=(IDiagnosticListener const&) = delete;

    virtual ~IDiagnosticListener() = default;

    virtual void onError(
        ::ip::IPEndpoint const& endpoint,
        SomeIpMessage const& message,
        SomeIpMessage::ReturnCode error)
        = 0;
};

} // namespace someip
