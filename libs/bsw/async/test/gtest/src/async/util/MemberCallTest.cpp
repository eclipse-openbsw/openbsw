// Copyright 2024 Accenture.

#include "async/util/MemberCall.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class AsyncMemberCallTest : public Test
{
public:
    MOCK_METHOD0(functionCall, void());
};

TEST_F(AsyncMemberCallTest, testMemberFunction)
{
    MemberCall<AsyncMemberCallTest, &AsyncMemberCallTest::functionCall> cut(*this);
    EXPECT_CALL(*this, functionCall());
    cut.execute();
}
} // namespace
