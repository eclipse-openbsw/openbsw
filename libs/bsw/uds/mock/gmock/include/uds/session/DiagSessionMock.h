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

#include "StubMock.h"
#include "uds/session/DiagSession.h"

#include <gmock/gmock.h>

namespace uds
{
template<class T>
class DiagSessionMockHelper
: public StubMock
, public T
{
public:
    DiagSessionMockHelper(bool pStub = true) : StubMock(pStub){};

    MOCK_METHOD(
        DiagReturnCode::Type, isTransitionPossible, (DiagSession::SessionType const targetSession));
    MOCK_METHOD(DiagSession&, getTransitionResult, (DiagSession::SessionType const targetSession));

    static DiagSessionMockHelper<T>& instance()
    {
        static DiagSessionMockHelper<T> instance;
        return instance;
    }
};
} // namespace uds
