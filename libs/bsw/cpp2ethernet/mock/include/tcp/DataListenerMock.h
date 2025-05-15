// Copyright 2025 Accenture.

#pragma once

#include "tcp/IDataListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct DataListenerMock : public IDataListener
{
    MOCK_METHOD(void, dataReceived, (uint16_t));
    MOCK_METHOD(void, connectionClosed, (ErrorCode));
};

} // namespace tcp
