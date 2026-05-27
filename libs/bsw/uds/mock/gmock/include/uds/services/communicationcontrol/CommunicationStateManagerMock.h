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

#include "uds/ICommunicationStateListener.h"
#include "uds/ICommunicationSubStateListener.h"
#include "uds/services/communicationcontrol/ICommunicationStateManager.h"

#include <gmock/gmock.h>

namespace uds
{
class CommunicationStateManagerMock : public ICommunicationStateManager
{
public:
    MOCK_METHOD(
        void, addCommunicationStateListener, (ICommunicationStateListener & listener), (override));

    MOCK_METHOD(
        void,
        removeCommunicationStateListener,
        (ICommunicationStateListener & listener),
        (override));

    MOCK_METHOD(
        void,
        addCommunicationSubStateListener,
        (ICommunicationSubStateListener & listener),
        (override));

    MOCK_METHOD(
        void,
        removeCommunicationSubStateListener,
        (ICommunicationSubStateListener & listener),
        (override));

    MOCK_METHOD(
        ICommunicationStateListener::CommunicationState,
        getCommunicationState,
        (),
        (const, override));

    MOCK_METHOD(
        void,
        setCommunicationState,
        (ICommunicationStateListener::CommunicationState state),
        (override));

    MOCK_METHOD(void, resetCommunicationSubState, (), (override));
};

} // namespace uds
