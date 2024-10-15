// Copyright 2025 Accenture.

#pragma once

#include "tcp/IDataSendNotificationListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct DataSendNotificationListenerMock : public IDataSendNotificationListener
{
    MOCK_METHOD2(dataSent, void(uint16_t, SendResult));
};

} // namespace tcp
