// Copyright 2024 Accenture.

#include "bsp/background/BackgroundJobHandlerMock.h"
#include "bsp/can/CanPhyMock.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::bsp;
using namespace testing;

TEST(ICanPhy, ICanPhyDefaultConstructorIsCalled) { can::CanPhyMock canPhy; }

} // anonymous namespace
