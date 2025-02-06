// Copyright 2024 Accenture.

#include <console/AsyncCommandWrapper.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/LifecycleControlCommand.h>

namespace systems
{
class SafetySystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit SafetySystem(
        ::async::ContextType context, ::lifecycle::ILifecycleManager& lifecycleManager);
    SafetySystem(SafetySystem const&)            = delete;
    SafetySystem& operator=(SafetySystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;
    void cyclic();

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;

    ::lifecycle::LifecycleControlCommand _lifecycleControlCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForLifecycleControlCommand;
};

} // namespace systems
