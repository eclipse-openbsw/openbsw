// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "lifecycle/StaticBsp.h"

#include "bsp/timer/SystemTimer.h"
#include "bsp/uart/UartConfig.h"

namespace platform
{

void StaticBsp::init()
{
    initSystemTimer();
    bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL).init();
}

} // namespace platform
