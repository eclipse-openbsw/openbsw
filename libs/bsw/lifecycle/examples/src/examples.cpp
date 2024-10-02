// Copyright 2024 Accenture.

#include "example.h"
#include "lifecycle/LifecycleManager.h"
#include "lifecycle/SingleContextLifecycleComponent.h"

#include <async/Types.h>

// EXAMPLE_START lifecycle_component_definition
// #include <lifecycle/SingleContextLifecycleComponent.h>
// #include <async/Types.h>

namespace lifecycle
{
class ComponentA : public ::lifecycle::SingleContextLifecycleComponent
{
public:
    // Inheriting from `SingleContextLifecycleComponent` makes all transitions be
    // scheduled within the same `context`.
    explicit ComponentA(::async::ContextType const context)
    : ::lifecycle::SingleContextLifecycleComponent(context)
    {}

    void init() override
    {
        // Perform component specific initialization. Will only be called when this component's
        // runlevel is first reached.

        transitionDone();
    }

    void run() override
    {
        // Perform tasks to run the component.

        transitionDone();
    }

    void shutdown() override
    {
        // Perform shutdown sequence of the component.

        transitionDone();
    }
};

// EXAMPLE_END lifecycle_component_definition

class ComponentB : public ::lifecycle::SingleContextLifecycleComponent
{
public:
    explicit ComponentB(::async::ContextType const context)
    : ::lifecycle::SingleContextLifecycleComponent(context)
    {}

    void init() override { transitionDone(); }

    void run() override { transitionDone(); }

    void shutdown() override { transitionDone(); }
};

class ComponentC : public ::lifecycle::SingleContextLifecycleComponent
{
public:
    explicit ComponentC(::async::ContextType const context)
    : ::lifecycle::SingleContextLifecycleComponent(context)
    {}

    void init() override { transitionDone(); }

    void run() override { transitionDone(); }

    void shutdown() override { transitionDone(); }
};

uint32_t getSystemTimeUs32Bit() { return 14; }

void runTheLifecycle()
{
    // EXAMPLE_START lifecycle_manager
    // #include <lifecycle/LifecycleManager.h>
    // #include <async/Types.h>

    ::async::ContextType const managerContext(1);
    ::async::ContextType const ethernetContext(2);
    ::async::ContextType const diagnosisContext(3);

    size_t const MAX_NUM_COMPONENTS           = 3;
    size_t const MAX_NUM_LEVELS               = 2;
    size_t const MAX_NUM_COMPONENTS_PER_LEVEL = 2;
    ::lifecycle::declare::
        LifecycleManager<MAX_NUM_COMPONENTS, MAX_NUM_LEVELS, MAX_NUM_COMPONENTS_PER_LEVEL>
            lifecycleManager(
                managerContext,
                ::lifecycle::LifecycleManager::GetTimestampType::create<&getSystemTimeUs32Bit>());

    // Transitions of components A and C are executed mutually exclusively.
    ComponentA componentA(ethernetContext);
    ComponentB componentB(diagnosisContext);
    ComponentC componentC(ethernetContext);

    // Components A and B are at the same run level.
    lifecycleManager.addComponent("component A", componentA, 1);
    lifecycleManager.addComponent("component B", componentB, 1);
    lifecycleManager.addComponent("component C", componentC, 2);

    lifecycleManager.transitionToLevel(2); // `init()` then `run()` of all components are called
    lifecycleManager.transitionToLevel(1); // `shutdown()` of C is called
    lifecycleManager.transitionToLevel(2); // only `run()` of C is called, `init()` already called
    lifecycleManager.transitionToLevel(0); // `shutdown()` is called on all components
    // EXAMPLE_END lifecycle_manager
}
} // namespace lifecycle
