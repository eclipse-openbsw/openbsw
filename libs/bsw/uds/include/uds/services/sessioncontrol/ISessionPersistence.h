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

#include "platform/estdint.h"

namespace uds
{
class DiagnosticSessionControl;

class ISessionPersistence
{
public:
    virtual void readSession(DiagnosticSessionControl& sessionControl) = 0;

    virtual void writeSession(DiagnosticSessionControl& sessionControl, uint8_t session) = 0;
};

} // namespace uds
