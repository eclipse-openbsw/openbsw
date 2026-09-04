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

#include "someip/IDiagnosticListener.h"

#include <gmock/gmock.h>

namespace someip
{
class DiagnosticListenerMock : public IDiagnosticListener
{
public:
    MOCK_METHOD(
        void,
        onError,
        (::ip::IPEndpoint const& endpoint,
         SomeIpMessage const& message,
         SomeIpMessage::ReturnCode error));
};

} // namespace someip
