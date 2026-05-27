/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/util/MemberCall.h"

#include "async/AsyncMock.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class AsyncMemberCallTest : public Test
{
public:
    MOCK_METHOD(void, functionCall, ());
    async::AsyncMock _asyncMock;
};

TEST_F(AsyncMemberCallTest, testMemberFunction)
{
    MemberCall<AsyncMemberCallTest, &AsyncMemberCallTest::functionCall> cut(*this);
    EXPECT_CALL(*this, functionCall());
    EXPECT_CALL(_asyncMock, execute(0, _))
        .Times(1)
        .WillOnce([this](ContextType const context, RunnableType& runnable)
                  { runnable.execute(); });
    execute(0, cut);
}
} // namespace
