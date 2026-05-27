/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
