// Copyright 2025 Accenture.

#pragma once

#include "tcp/IDataListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct DataListenerMock : public IDataListener
{
    MOCK_METHOD1(dataReceived, void(uint16_t));
    MOCK_METHOD1(connectionClosed, void(ErrorCode));
};

} // namespace tcp
