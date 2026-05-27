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

#include "uds/session/DiagSession.h"

namespace uds
{
class ApplicationDefaultSession : public DiagSession
{
public:
    ApplicationDefaultSession() : DiagSession(DEFAULT, 0x01U) {}

    DiagReturnCode::Type isTransitionPossible(DiagSession::SessionType targetSession) override;

    DiagSession& getTransitionResult(DiagSession::SessionType targetSession) override;
};

} /* namespace uds */
