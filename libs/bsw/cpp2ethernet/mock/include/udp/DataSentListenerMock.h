// Copyright 2025 Accenture.

#pragma once

#include "udp/IDataSentListener.h"

#include <gmock/gmock.h>

namespace udp
{
struct DataSentListenerMock : public IDataSentListener
{
    MOCK_METHOD(void, dataSent, (uint8_t const[], uint16_t));
};

} // namespace udp
