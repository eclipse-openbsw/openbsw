// Copyright 2025 Accenture.

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
