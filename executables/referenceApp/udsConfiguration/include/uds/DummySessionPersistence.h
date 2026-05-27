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

namespace uds
{
class DummySessionPersistence : public ISessionPersistence
{
public:
    void readSession(DiagnosticSessionControl&) override {}

    void writeSession(DiagnosticSessionControl&, uint8_t) override {}
};

} // namespace uds
