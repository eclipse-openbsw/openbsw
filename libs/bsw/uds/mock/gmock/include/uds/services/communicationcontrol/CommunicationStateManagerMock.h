// Copyright 2024 Accenture.

#ifndef GUARD_A09987FD_6AC4_4B20_83D9_95FFFFCF12E8
#define GUARD_A09987FD_6AC4_4B20_83D9_95FFFFCF12E8

#include "uds/ICommunicationStateListener.h"
#include "uds/ICommunicationSubStateListener.h"
#include "uds/services/communicationcontrol/ICommunicationStateManager.h"

#include <gmock/gmock.h>

namespace uds
{
class CommunicationStateManagerMock : public ICommunicationStateManager
{
public:
    MOCK_METHOD1(addCommunicationStateListener, void(ICommunicationStateListener& listener));

    MOCK_METHOD1(removeCommunicationStateListener, void(ICommunicationStateListener& listener));

    MOCK_METHOD1(addCommunicationSubStateListener, void(ICommunicationSubStateListener& listener));

    MOCK_METHOD1(
        removeCommunicationSubStateListener, void(ICommunicationSubStateListener& listener));

    MOCK_CONST_METHOD0(getCommunicationState, ICommunicationStateListener::CommunicationState());

    MOCK_METHOD1(
        setCommunicationState, void(ICommunicationStateListener::CommunicationState state));

    MOCK_METHOD0(resetCommunicationSubState, void());
};

} // namespace uds

#endif // GUARD_A09987FD_6AC4_4B20_83D9_95FFFFCF12E8
