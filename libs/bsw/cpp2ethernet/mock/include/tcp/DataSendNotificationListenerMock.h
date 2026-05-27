/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
