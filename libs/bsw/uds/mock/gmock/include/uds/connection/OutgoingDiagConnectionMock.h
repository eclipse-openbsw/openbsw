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
    MOCK_METHOD0(terminate, void());
    MOCK_METHOD0(timeoutOccured, void());
};

} // namespace uds

