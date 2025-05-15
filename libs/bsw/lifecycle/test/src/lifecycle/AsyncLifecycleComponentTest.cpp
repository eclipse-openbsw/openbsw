// Copyright 2024 Accenture.

#include "lifecycle/AsyncLifecycleComponent.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::lifecycle;
using namespace ::testing;

class AsyncLifecycleComponentMock : public AsyncLifecycleComponent
{
public:
    MOCK_METHOD(void, init, ());
    MOCK_METHOD(void, run, ());
    MOCK_METHOD(void, shutdown, ());

    using AsyncLifecycleComponent::setTransitionContext;
};

TEST(AsyncLifecycleComponentTest, testCompleteComponent)
{
    {
        StrictMock<AsyncLifecycleComponentMock> cut;
        {
            // default contexts
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
        {
            // defined contexts
            ::async::ContextType context1(1U);
            cut.setTransitionContext(ILifecycleComponent::Transition::Type::INIT, context1);
            ::async::ContextType context_1
                = cut.getTransitionContext(ILifecycleComponent::Transition::Type::INIT);
            EXPECT_EQ(context1, context_1);
            ::async::ContextType context2(2U);
            cut.setTransitionContext(ILifecycleComponent::Transition::Type::RUN, 2U);
            ::async::ContextType context_2
                = cut.getTransitionContext(ILifecycleComponent::Transition::Type::RUN);
            EXPECT_EQ(context2, context_2);
            ::async::ContextType context3(3U);
            cut.setTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN, 3U);
            ::async::ContextType context_3
                = cut.getTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN);
            EXPECT_EQ(context3, context_3);
        }
    }
    {
        StrictMock<AsyncLifecycleComponentMock> cut;
        {
            // default contexts
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
        ::async::ContextType context(2U);
        cut.setTransitionContext(context);
        {
            // the same context for all transitions
            ::async::ContextType context_1
                = cut.getTransitionContext(ILifecycleComponent::Transition::Type::INIT);
            EXPECT_EQ(context, context_1);
            context_1 = cut.getTransitionContext(ILifecycleComponent::Transition::Type::RUN);
            EXPECT_EQ(context, context_1);
            context_1 = cut.getTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN);
            EXPECT_EQ(context, context_1);
        }
    }
}

} // anonymous namespace
