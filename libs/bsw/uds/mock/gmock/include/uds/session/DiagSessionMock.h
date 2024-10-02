// Copyright 2024 Accenture.

#ifndef GUARD_FA3492E2_3BCE_4FAB_B55A_A9E1988C5F26
#define GUARD_FA3492E2_3BCE_4FAB_B55A_A9E1988C5F26

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
#endif /* GUARD_FA3492E2_3BCE_4FAB_B55A_A9E1988C5F26 */
