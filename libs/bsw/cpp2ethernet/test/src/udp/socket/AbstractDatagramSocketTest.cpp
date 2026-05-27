/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
