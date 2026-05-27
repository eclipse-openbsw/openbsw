/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "tcp/DataListenerMock.h"
#include "tcp/DataSendNotificationListenerMock.h"
#include "tcp/socket/AbstractSocketMock.h"

#include <gtest/gtest.h>

using namespace ::testing;

using namespace tcp;

namespace
{
TEST(AbstractSocketTest, DefaultConstructor)
{
    StrictMock<AbstractSocketMock> s;

    ASSERT_EQ(nullptr, s.getDataListener());
    ASSERT_EQ(nullptr, s.getSendNotificationListener());

    StrictMock<DataListenerMock> listener;

    s.setDataListener(&listener);
    ASSERT_EQ(&listener, s.getDataListener());

    StrictMock<DataSendNotificationListenerMock> sendListener;
    s.setSendNotificationListener(&sendListener);

    ASSERT_EQ(&sendListener, s.getSendNotificationListener());
}

} // anonymous namespace
