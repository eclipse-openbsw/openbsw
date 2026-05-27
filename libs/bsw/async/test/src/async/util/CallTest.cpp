/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/util/Call.h"

#include "async/AsyncMock.h"

#include <etl/closure.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class AsyncCallTest : public Test
{
public:
    MOCK_METHOD(void, functionCall, ());
    MOCK_METHOD(void, closureCall, (uint16_t, uint32_t));
    async::AsyncMock _asyncMock;
};

TEST_F(AsyncCallTest, testFunction)
{
    Function cut(Function::CallType::create<AsyncCallTest, &AsyncCallTest::functionCall>(*this));
    EXPECT_CALL(*this, functionCall());
    EXPECT_CALL(_asyncMock, execute(0, _))
        .Times(1)
        .WillOnce([](ContextType const /* context */, RunnableType& runnable)
                  { runnable.execute(); });
    execute(0, cut);
}

TEST_F(AsyncCallTest, testClosure)
{
    using TestClosure = ::etl::closure<void(uint16_t, uint32_t)>;
    Call<TestClosure> cut(TestClosure(
        TestClosure::delegate_type::create<AsyncCallTest, &AsyncCallTest::closureCall>(*this),
        1234U,
        3247834U));
    EXPECT_CALL(*this, closureCall(1234U, 3247834U));
    EXPECT_CALL(_asyncMock, execute(0, _))
        .Times(1)
        .WillOnce([](ContextType const /* context */, RunnableType& runnable)
                  { runnable.execute(); });
    execute(0, cut);
}
} // namespace
