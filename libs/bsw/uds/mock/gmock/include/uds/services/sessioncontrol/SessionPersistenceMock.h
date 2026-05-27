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

#include "uds/services/sessioncontrol/ISessionPersistence.h"

#include <gmock/gmock.h>

namespace uds
{
class SessionPersistenceMock : public ISessionPersistence
{
public:
    SessionPersistenceMock() {}

    MOCK_METHOD(void, readSession, (DiagnosticSessionControl & sessionControl), (override));
    MOCK_METHOD(
        void,
        writeSession,
        (DiagnosticSessionControl & sessionControl, uint8_t session),
        (override));
};

} // namespace uds
