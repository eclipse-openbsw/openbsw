// Copyright 2024 Accenture.

#define MINIMUM_STACK_SIZE 1024

#include "async/TaskInitializer.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::async::internal;
using namespace ::testing;

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterTaskInitializer
 * \desc: The test is to prove that Stack object is constructed with correct size
 */
TEST(TaskInitializerStackTest, testStack)
{
    EXPECT_EQ(adjustStackSize(0), 0);
    Stack<0> stackZero;
    EXPECT_EQ(stackZero.size(), 0);

    EXPECT_EQ(adjustStackSize(386), MINIMUM_STACK_SIZE);
    Stack<386> stackMin;
    EXPECT_EQ(stackMin.size() * sizeof(StackType_t), MINIMUM_STACK_SIZE);

    EXPECT_EQ(adjustStackSize(MINIMUM_STACK_SIZE), MINIMUM_STACK_SIZE);
    Stack<MINIMUM_STACK_SIZE> stackMinExact;
    EXPECT_EQ(stackMinExact.size() * sizeof(StackType_t), MINIMUM_STACK_SIZE);

    EXPECT_EQ(adjustStackSize(2048), 2048);
    Stack<2048> stackMoreThanMin;
    EXPECT_EQ(stackMoreThanMin.size() * sizeof(StackType_t), 2048);
}

} // namespace
