// Copyright 2024 Accenture.

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
