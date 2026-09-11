/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "console/AsyncCommandWrapper.h"
#include "inputManager/DigitalInputTester.h"
#include "lifecycle/SingleContextLifecycleComponent.h"
#include "lifecycle/StaticBsp.h"
#include "outputManager/OutputTester.h"
#include "outputPwm/OutputPwmTester.h"
#include "tester/AnalogTester.h"

namespace systems
{

class BspSystem
: public ::lifecycle::SingleContextLifecycleComponent
, private ::async::RunnableType
{
public:
    BspSystem(::async::ContextType context, StaticBsp& staticBsp);

    void init() override;
    void run() override;
    void shutdown() override;

    void execute() override;

    void cyclic();

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
    StaticBsp& _staticBsp;
    bios::AnalogTester _analogTester;
    bios::OutputPwmTester _outputPwmTester;
    bios::DigitalInputTester _digitalInputTester;
    bios::OutputTester _outputTester;
    ::console::AsyncCommandWrapper _asyncCommandWrapper_for_analogTester;
    ::console::AsyncCommandWrapper _asyncOutputPwmTester;
    ::console::AsyncCommandWrapper _asyncDigitalInputTester;
    ::console::AsyncCommandWrapper _asyncOutputTester;
};

} // namespace systems
