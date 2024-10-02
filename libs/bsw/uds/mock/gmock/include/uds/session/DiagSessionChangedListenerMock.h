// Copyright 2024 Accenture.

#ifndef GUARD_E0FD2B6E_99E1_4EE1_A7B5_9F9A8F0144B1
#define GUARD_E0FD2B6E_99E1_4EE1_A7B5_9F9A8F0144B1

#include "uds/session/IDiagSessionChangedListener.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagSessionChangedListenerMock : public IDiagSessionChangedListener
{
public:
    MOCK_METHOD1(diagSessionChanged, void(DiagSession const& session));
    MOCK_METHOD1(diagSessionResponseSent, void(uint8_t const responseCode));
};

} // namespace uds

#endif // GUARD_E0FD2B6E_99E1_4EE1_A7B5_9F9A8F0144B1
