/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "tcp/socket/AbstractServerSocketMock.h"
#include "tcp/socket/SocketProvidingConnectionListenerMock.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace tcp;

TEST(AbstractServerSocketTest, DefaultConstructor)
{
    StrictMock<AbstractServerSocketMock> s;

    ASSERT_EQ(0U, s.getLocalPort());

    s.setPort(10U);
    ASSERT_EQ(10U, s.getLocalPort());

    StrictMock<SocketProvidingConnectionListenerMock> listener;
    s.setSocketProvidingConnectionListener(listener);

    ASSERT_EQ(&listener, &s.getSocketProvidingConnectionListener());
}

TEST(AbstractServerSocketTest, PortAndListenerConstructor)
{
    StrictMock<SocketProvidingConnectionListenerMock> listener;
    StrictMock<AbstractServerSocketMock> s(10U, listener);

    ASSERT_EQ(10U, s.getLocalPort());
    ASSERT_EQ(&listener, &s.getSocketProvidingConnectionListener());
}
