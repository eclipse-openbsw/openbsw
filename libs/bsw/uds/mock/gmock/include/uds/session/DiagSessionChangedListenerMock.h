// Copyright 2024 Accenture.

#pragma once

#include "uds/session/IDiagSessionChangedListener.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagSessionChangedListenerMock : public IDiagSessionChangedListener
{
public:
    MOCK_METHOD(void, diagSessionChanged, (DiagSession const& session), (override));
    MOCK_METHOD(void, diagSessionResponseSent, (uint8_t const responseCode), (override));
};

} // namespace uds
