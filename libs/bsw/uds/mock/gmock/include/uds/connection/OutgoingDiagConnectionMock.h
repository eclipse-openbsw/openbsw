// Copyright 2024 Accenture.

#pragma once

#include "uds/connection/OutgoingDiagConnection.h"

#include <gmock/gmock.h>

namespace uds
{
using namespace ::testing;

class OutgoingDiagConnectionMock : public OutgoingDiagConnection
{
public:
    MOCK_METHOD(void, terminate, (), (override));
    MOCK_METHOD(void, timeoutOccurred, (), (override));
};

} // namespace uds
