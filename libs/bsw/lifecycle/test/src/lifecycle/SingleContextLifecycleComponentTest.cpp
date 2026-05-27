/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/SingleContextLifecycleComponent.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::lifecycle;
using namespace ::testing;

class SingleContextLifecycleComponentMock : public SingleContextLifecycleComponent
{
public:
    explicit SingleContextLifecycleComponentMock(::async::ContextType const context)
    : SingleContextLifecycleComponent(context)
    {}

    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
};

TEST(SingleContextLifecycleComponentTest, testCompleteComponent)
{
    {
        StrictMock<SingleContextLifecycleComponentMock> cut(::async::CONTEXT_INVALID);
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
        ::async::ContextType context(2U);
        StrictMock<SingleContextLifecycleComponentMock> cut(context);
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

} // anonymous namespace
