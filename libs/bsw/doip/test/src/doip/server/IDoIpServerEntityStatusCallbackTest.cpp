/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/IDoIpServerEntityStatusCallback.h"

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(IDoIpServerEntityStatusCallbackTest, TestEntityStatus)
{
    // constructor
    IDoIpServerEntityStatusCallback::EntityStatus cut(0x12U, 0x34U, 0x56U, 0x12345678U);
    EXPECT_EQ(0x12U, cut._nodeType);
    EXPECT_EQ(0x34U, cut._maxConnectionCount);
    EXPECT_EQ(0x56U, cut._connectionCount);
    EXPECT_EQ(0x12345678U, cut._maxDataSize);
}

} // namespace test
} // namespace doip
