// Copyright 2025 Accenture.

#include "udp/DataListenerMock.h"
#include "udp/DataSentListenerMock.h"
#include "udp/socket/AbstractDatagramSocketMock.h"

#include <gtest/gtest.h>

using namespace ::testing;

using namespace udp;

namespace
{
TEST(AbstractSocketTest, DefaultConstructor)
{
    StrictMock<AbstractDatagramSocketMock> s;

    ASSERT_EQ(nullptr, s.getDataListener());
    ASSERT_EQ(nullptr, s.getDataSentListener());

    StrictMock<DataListenerMock> listener;

    s.setDataListener(&listener);
    ASSERT_EQ(&listener, s.getDataListener());

    StrictMock<DataSentListenerMock> sendListener;
    s.setDataSentListener(&sendListener);

    ASSERT_EQ(&sendListener, s.getDataSentListener());
}

} // anonymous namespace
