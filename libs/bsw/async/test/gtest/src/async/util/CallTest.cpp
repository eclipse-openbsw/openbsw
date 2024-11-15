// Copyright 2024 Accenture.

#include "async/util/Call.h"

#include "async/AsyncMock.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class AsyncCallTest : public Test
{
public:
    MOCK_METHOD0(functionCall, void());
    MOCK_METHOD2(closureCall, void(uint16_t, uint32_t));
    async::AsyncMock _asyncMock;
};

TEST_F(AsyncCallTest, testFunction)
{
    Function cut([this]() { functionCall(); });
    EXPECT_CALL(*this, functionCall());
    EXPECT_CALL(_asyncMock, execute(0, _))
        .Times(1)
        .WillOnce([](ContextType const /* context */, RunnableType& runnable)
                  { runnable.execute(); });
    execute(0, cut);
}

TEST_F(AsyncCallTest, testClosure)
{
    auto l            = [&]() { closureCall(1234U, 3247834U); };
    using TestClosure = decltype(l);
    Call<TestClosure> cut{TestClosure(l)};
    EXPECT_CALL(*this, closureCall(1234U, 3247834U));
    EXPECT_CALL(_asyncMock, execute(0, _))
        .Times(1)
        .WillOnce([](ContextType const /* context */, RunnableType& runnable)
                  { runnable.execute(); });
    execute(0, cut);
}
} // namespace
