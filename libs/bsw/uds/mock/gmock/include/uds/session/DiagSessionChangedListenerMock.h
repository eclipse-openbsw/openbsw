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

#include "uds/session/IDiagSessionChangedListener.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagSessionChangedListenerMock : public IDiagSessionChangedListener
{
public:
    MOCK_METHOD(void, diagSessionChanged, (DiagSession const& session), (override));
    MOCK_METHOD(void, diagSessionResponseSent, (uint8_t const responseCode), (override));
};

} // namespace uds
