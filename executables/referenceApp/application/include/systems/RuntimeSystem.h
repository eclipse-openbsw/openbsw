// Copyright 2024 Accenture.

#ifndef GUARD_EB2D422D_3098_4A56_A3FA_884EB2AA7B1C
#define GUARD_EB2D422D_3098_4A56_A3FA_884EB2AA7B1C

#include <async/Async.h>
#include <async/IRunnable.h>
#include <console/AsyncCommandWrapper.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/StatisticsCommand.h>

namespace systems
{

class RuntimeSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit RuntimeSystem(
        ::async::ContextType context, ::async::AsyncBinding::RuntimeMonitorType& runtimeMonitor);
    RuntimeSystem(RuntimeSystem const&)            = delete;
    RuntimeSystem& operator=(RuntimeSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;

    ::lifecycle::StatisticsCommand _statisticsCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForStatisticsCommand;
};

} // namespace systems

#endif /* GUARD_EB2D422D_3098_4A56_A3FA_884EB2AA7B1C */
