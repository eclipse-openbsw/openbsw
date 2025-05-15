// Copyright 2024 Accenture.

#include "lifecycle/SimpleLifecycleComponent.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::lifecycle;
using namespace ::testing;

class SimpleLifecycleComponentMock : public SimpleLifecycleComponent
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
};

TEST(SimpleLifecycleComponentTest, testCompleteComponent)
{
    StrictMock<SimpleLifecycleComponentMock> cut;
    EXPECT_EQ(
        ::async::CONTEXT_INVALID,
        cut.getTransitionContext(ILifecycleComponent::Transition::Type::INIT));
    EXPECT_EQ(
        ::async::CONTEXT_INVALID,
        cut.getTransitionContext(ILifecycleComponent::Transition::Type::RUN));
    EXPECT_EQ(
        ::async::CONTEXT_INVALID,
        cut.getTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN));
}

} // anonymous namespace
