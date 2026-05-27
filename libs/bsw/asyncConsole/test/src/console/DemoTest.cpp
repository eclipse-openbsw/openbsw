/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "console/AsyncCommandWrapper.h"
#include "demo/DemoCommand.h"

#include <async/AsyncBinding.h>

#include <gmock/gmock.h>

namespace
{

using namespace ::util::stream;
using namespace ::testing;

TEST(ExampleTest, AsyncCommandWrapperTest)
{
    // EXAMPLE_START AsyncCommandWrapper
    static ::demo::DemoCommand demoCommand{};
    static ::console::AsyncCommandWrapper const wrapper(
        demoCommand, ::async::AsyncBinding::AdapterType::TASK_IDLE);
    // EXAMPLE_END AsyncCommandWrapper
}
} // namespace
