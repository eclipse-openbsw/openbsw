// Copyright 2024 Accenture.

#pragma once

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
