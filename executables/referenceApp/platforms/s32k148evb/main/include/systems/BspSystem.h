// Copyright 2024 Accenture.

#ifndef GUARD_60DA69EC_9622_4FC2_BB0B_C4C79C5CBD25
#define GUARD_60DA69EC_9622_4FC2_BB0B_C4C79C5CBD25

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
    BspSystem(::async::ContextType const context, StaticBsp& staticBsp);

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

#endif /* GUARD_60DA69EC_9622_4FC2_BB0B_C4C79C5CBD25 */
