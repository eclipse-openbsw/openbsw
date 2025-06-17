// Copyright 2024 Accenture.

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
