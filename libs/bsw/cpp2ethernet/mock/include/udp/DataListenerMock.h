// Copyright 2025 Accenture.

#pragma once

#include "udp/IDataListener.h"

#include <gmock/gmock.h>

namespace udp
{
struct DataListenerMock : public IDataListener
{
    MOCK_METHOD5(
        dataReceived,
        void(AbstractDatagramSocket&, ::ip::IPAddress, uint16_t, ::ip::IPAddress, uint16_t));
};

} // namespace udp
