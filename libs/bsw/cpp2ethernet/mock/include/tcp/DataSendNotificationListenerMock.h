// Copyright 2025 Accenture.

#pragma once

#include "tcp/IDataSendNotificationListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct DataSendNotificationListenerMock : public IDataSendNotificationListener
{
    MOCK_METHOD(void, dataSent, (uint16_t, SendResult));
};

} // namespace tcp
