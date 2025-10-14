// Copyright 2024 Accenture.

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
    MOCK_METHOD0(functionCall, void());
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
