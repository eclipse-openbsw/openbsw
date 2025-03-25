// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleComponent.h"
#include "lifecycle/ILifecycleComponentCallback.h"
#include "lifecycle/ILifecycleListener.h"
#include "lifecycle/ILifecycleManager.h"

#include <async/Async.h>

#include <estd/array.h>
#include <estd/forward_list.h>
#include <estd/functional.h>
#include <estd/slice.h>
#include <estd/vector.h>

namespace lifecycle
{
/// The LifecycleManager is the central piece of the lifecycle module. It stores the component
/// registry and triggers initialization, running, and shutdown of these components. It is declared
/// using a `declare::LifecycleManager` with static capacities of components, runlevels, and
/// components per runlevel.
class LifecycleManager
: public ILifecycleManager
, private ILifecycleComponentCallback
, private ::async::RunnableType
{
public:
    struct ComponentInfo
    {
        /// A name used during logging.
        char const* _name               = nullptr;
        /// Reference to the managed component.
        ILifecycleComponent* _component = nullptr;
        /// The durations of passed run state transition.
        uint32_t _transitionTimes[ILifecycleComponent::Transition::COUNT]{};
        /// A bool that stores whether this component is currently performing a transition. I. e. a
        /// transition has been triggered by the LifecycleManager but `transitionDone()` has not yet
        /// been called.
        bool _isTransitionPending = false;
        /// The type of transition that was most recently started.
        ILifecycleComponent::Transition::Type _lastTransition
            = ILifecycleComponent::Transition::Type::INIT;
    };

    /// System time callback.
    using GetTimestampType = ::estd::function<uint32_t()>;

    /// Adds a lifecycle `component` to be managed by this LifecycleManager. It is registered at the
    /// given `level`. Additionally `name` is used during logging to identify this component.
    ///
    /// The component's lifetime must cover that of the manager.
    void addComponent(char const* name, ILifecycleComponent& component, uint8_t level);

    /// Returns the current number of registered components.
    size_t getComponentCount() const { return static_cast<size_t>(_componentCount); }

    /// Returns the component info at internal index `idx`.
    ComponentInfo const& getComponentInfo(size_t const idx) const { return _componentInfos[idx]; }

    /// Returns the highest registered component's level.
    uint8_t getLevelCount() const override { return _levelCount; }

    /// Transition to `level`.
    ///
    /// If the targeted `level` is higher than the current level, this will initialize and then run
    /// all components in each runlevel, up to the target `level`. If a lower level is targeted for
    /// a transition, all components in levels above the target `level` are shut down, in descending
    /// order of their runlevel.
    ///
    /// Note that even if a component is shut down and run again by a sequence of calls to
    /// `transitionToLevel()`, it is initialized at most once by the LifecycleManager the first time
    /// its run level is reached.
    void transitionToLevel(uint8_t level) override;
    /// Add a `listener` to be notified of any transitions.
    void addLifecycleListener(ILifecycleListener& listener) override;
    /// Remove a previously registered `listener`.
    void removeLifecycleListener(ILifecycleListener& listener) override;

protected:
    struct ComponentTransitionExecutor : public ::async::RunnableType
    {
        ComponentTransitionExecutor(
            ILifecycleComponent& component,
            uint8_t const componentIndex,
            ILifecycleComponent::Transition::Type const transition)
        : _component(component)
        , _componentIndex(componentIndex)
        , _transition(transition)
        , _isPending(true)
        {}

        virtual ~ComponentTransitionExecutor() = default;

        void execute() override;

        ILifecycleComponent& _component;
        uint8_t _componentIndex;
        ILifecycleComponent::Transition::Type _transition;
        bool _isPending = true;
    };

    using ComponentInfoSliceType                = ::estd::slice<ComponentInfo>;
    using ComponentTransitionExecutorVectorType = ::estd::vector<ComponentTransitionExecutor>;
    using LevelIndexSliceType                   = ::estd::slice<uint8_t>;

    LifecycleManager(
        ::async::ContextType transitionContext,
        GetTimestampType getTimestamp,
        ComponentInfoSliceType const& componentInfoSlice,
        ComponentTransitionExecutorVectorType& componentTransitionExecutorVector,
        LevelIndexSliceType const& levelIndexSlice);

    ~LifecycleManager() override = default;

private:
    using LifecycleListenerListType = ::estd::forward_list<ILifecycleListener>;

    void transitionDone(ILifecycleComponent& component) override;

    void execute() override;

    bool checkLevelTransitionDone();

    size_t getLevelComponentCount(uint8_t const level) const
    {
        return _levelIndices[static_cast<size_t>(level)]
               - _levelIndices[static_cast<size_t>(level - 1U)];
    }

    static char const* getTransitionString(ILifecycleComponent::Transition::Type transition);

    ComponentInfoSliceType _componentInfos;
    ComponentTransitionExecutorVectorType& _componentTransitionExecutors;
    LevelIndexSliceType _levelIndices;
    LifecycleListenerListType _listeners;
    GetTimestampType _getTimestamp;
    uint32_t _transitionStartTimestamp = 0;
    ::async::ContextType _transitionContext;
    bool _isTransitionPending                         = false;
    ILifecycleComponent::Transition::Type _transition = ILifecycleComponent::Transition::Type::INIT;
    uint8_t _transitionLevel                          = 0;
    uint8_t _componentCount                           = 0;
    uint8_t _levelCount                               = 0;
    uint8_t _initLevelCount                           = 0;
    uint8_t _currentLevel                             = 0;
    uint8_t _nextLevel                                = 0;
};

namespace declare
{
template<size_t MAX_NUM_COMPONENTS, size_t MAX_NUM_LEVELS, size_t MAX_NUM_COMPONENTS_PER_LEVEL>
class LifecycleManager : public ::lifecycle::LifecycleManager
{
public:
    LifecycleManager(
        ::async::ContextType const transitionContext, GetTimestampType const getTimestamp)
    : ::lifecycle::LifecycleManager(
        transitionContext,
        getTimestamp,
        _componentInfoArray,
        _componentTransitionExecutorVector,
        _levelIndexArray)
    {}

private:
    ::estd::array<ComponentInfo, MAX_NUM_COMPONENTS> _componentInfoArray{};
    ::estd::declare::vector<ComponentTransitionExecutor, MAX_NUM_COMPONENTS_PER_LEVEL>
        _componentTransitionExecutorVector{};
    ::estd::array<uint8_t, MAX_NUM_LEVELS + 1> _levelIndexArray{};
};

} // namespace declare
} // namespace lifecycle

