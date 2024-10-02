// Copyright 2024 Accenture.

#include "estd/typed_mem.h"

#include "estd/slice.h"

#include <gmock/gmock.h>

#include <array>

namespace
{
class ILifecycleSystem
{};

class LifecycleManager
{
public:
    void addComponent(char const* name, ILifecycleSystem& system, size_t runlevel){};
} lifecycleManager;

// Some classes just for demonstration purposes.
class StaticBsp
{
} staticBsp;

class CanSystem : public ILifecycleSystem
{
public:
    CanSystem(uint8_t taskContext, StaticBsp& staticBsp) {}
};

constexpr uint8_t TASK_CAN{1};

// [EXAMPLE_BEGIN:typed_mem_lifecycle]

// Create the memory for a CanSystem.
estd::typed_mem<CanSystem> canSystem;

void initLifecycle()
{
    // Construct CanSystem and add it to lifecycleManager at runlevel 1.
    lifecycleManager.addComponent("can", canSystem.emplace(TASK_CAN, staticBsp), 1);
}

// [EXAMPLE_END:typed_mem_lifecycle]

} // namespace

TEST(Typed_mem, run_examples) { initLifecycle(); }
