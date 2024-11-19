// Copyright 2024 Accenture.

#ifndef GUARD_B7AFD60E_D676_4056_BE5F_EDB78DF1C357
#define GUARD_B7AFD60E_D676_4056_BE5F_EDB78DF1C357

#include <async/Async.h>
#include <async/IRunnable.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/LifecycleControlCommand.h>
#ifdef PLATFORM_SUPPORT_CAN
#include "app/CanDemoListener.h"

#include <can/console/CanCommand.h>
#include <console/AsyncCommandWrapper.h>
#include <systems/ICanSystem.h>
#endif

namespace systems
{

class DemoSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit DemoSystem(
        ::async::ContextType context,
        ::lifecycle::ILifecycleManager& lifecycleManager
#ifdef PLATFORM_SUPPORT_CAN
        ,
        ::can::ICanSystem& canSystem
#endif
    );

    DemoSystem(DemoSystem const&)            = delete;
    DemoSystem& operator=(DemoSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

    void cyclic();

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
#ifdef PLATFORM_SUPPORT_CAN
    ::can::ICanSystem& _canSystem;
    ::can::CanDemoListener _canDemoListener;
    ::can::CanCommand _canCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForCanCommand;
#endif
};

} // namespace systems

#endif /* GUARD_B7AFD60E_D676_4056_BE5F_EDB78DF1C357 */
