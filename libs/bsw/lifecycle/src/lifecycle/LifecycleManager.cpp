// Copyright 2024 Accenture.

#include "lifecycle/LifecycleManager.h"

#include "lifecycle/LifecycleLogger.h"

#include <estd/assert.h>

namespace lifecycle
{
using ::util::logger::LIFECYCLE;
using ::util::logger::Logger;

LifecycleManager::LifecycleManager(
    ::async::ContextType const transitionContext,
    GetTimestampType const getTimestamp,
    ComponentInfoSliceType const& componentInfoSlice,
    ComponentTransitionExecutorVectorType& componentTransitionExecutorVector,
    LevelIndexSliceType const& levelIndexSlice)
: _componentInfos(componentInfoSlice)
, _componentTransitionExecutors(componentTransitionExecutorVector)
, _levelIndices(levelIndexSlice)
, _listeners()
, _getTimestamp(getTimestamp)
, _transitionContext(transitionContext)
{}

void LifecycleManager::addComponent(
    char const* const name, ILifecycleComponent& component, uint8_t const level)
{
    estd_assert(_componentCount < _componentInfos.size());
    estd_assert(level > 0U);
    estd_assert(level < _levelIndices.size());
    estd_assert(level >= _levelCount);

    while (_levelCount < level)
    {
        ++_levelCount;
        _levelIndices[static_cast<size_t>(_levelCount)]
            = _levelIndices[static_cast<size_t>(_levelCount) - 1U];
    }

    estd_assert(getLevelComponentCount(level) < _componentTransitionExecutors.max_size());

    auto& componentInfo      = _componentInfos[static_cast<size_t>(_componentCount)];
    componentInfo._name      = name;
    componentInfo._component = &component;
    ++_levelIndices[static_cast<size_t>(_levelCount)];
    ++_componentCount;
    component.initCallback(*this);
}

void LifecycleManager::transitionToLevel(uint8_t const level)
{
    _nextLevel = (static_cast<size_t>(level) < _levelIndices.size())
                     ? level
                     : static_cast<uint8_t>(_levelIndices.size() - 1U);
    ::async::execute(_transitionContext, *this);
}

void LifecycleManager::addLifecycleListener(ILifecycleListener& listener)
{
    ::async::LockType const lock;
    _listeners.push_front(listener);
}

void LifecycleManager::removeLifecycleListener(ILifecycleListener& listener)
{
    ::async::LockType const lock;
    _listeners.remove(listener);
}

void LifecycleManager::transitionDone(ILifecycleComponent& component)
{
    for (auto& executor : _componentTransitionExecutors)
    {
        if (&component == &executor._component)
        {
            executor._isPending = false;
            ComponentInfo& componentInfo
                = _componentInfos[static_cast<size_t>(executor._componentIndex)];
            componentInfo._isTransitionPending = false;
            componentInfo._transitionTimes[static_cast<uint8_t>(executor._transition)]
                = _getTimestamp() - _transitionStartTimestamp;
            Logger::debug(
                LIFECYCLE,
                "%s %s done",
                getTransitionString(executor._transition),
                componentInfo._name);
            ::async::execute(_transitionContext, *this);
            break;
        }
    }
}

void LifecycleManager::execute()
{
    if ((!checkLevelTransitionDone())
        || ((_currentLevel == _nextLevel) && (_initLevelCount >= _nextLevel)))
    {
        return;
    }
    if (_nextLevel < _currentLevel)
    {
        _transitionLevel = _currentLevel;
        --_currentLevel;
        _transition = ILifecycleComponent::Transition::Type::SHUTDOWN;
    }
    else if ((_initLevelCount < _nextLevel) && (_initLevelCount == _currentLevel))
    {
        ++_initLevelCount;
        _transitionLevel = _initLevelCount;
        _transition      = ILifecycleComponent::Transition::Type::INIT;
    }
    else
    {
        ++_currentLevel;
        _transitionLevel = _currentLevel;
        _transition      = ILifecycleComponent::Transition::Type::RUN;
    }
    _isTransitionPending      = true;
    _transitionStartTimestamp = _getTimestamp();
    Logger::info(LIFECYCLE, "%s level %d", getTransitionString(_transition), _transitionLevel);
    for (uint8_t componentIndex = _levelIndices[static_cast<size_t>(_transitionLevel) - 1U];
         componentIndex < _levelIndices[static_cast<size_t>(_transitionLevel)];
         ++componentIndex)
    {
        auto& componentInfo = _componentInfos[static_cast<size_t>(componentIndex)];
        ComponentTransitionExecutor& transitionExecutor
            = _componentTransitionExecutors.emplace_back().construct(
                *componentInfo._component, componentIndex, _transition);
        componentInfo._isTransitionPending = true;
        componentInfo._lastTransition      = _transition;
        ::async::ContextType transitionContext
            = componentInfo._component->getTransitionContext(_transition);
        if (transitionContext == ::async::CONTEXT_INVALID)
        {
            transitionContext = _transitionContext;
        }
        Logger::info(LIFECYCLE, "%s %s", getTransitionString(_transition), componentInfo._name);
        ::async::execute(transitionContext, transitionExecutor);
    }
    if (_componentTransitionExecutors.empty())
    {
        ::async::execute(_transitionContext, *this);
    }
}

bool LifecycleManager::checkLevelTransitionDone()
{
    if (!_isTransitionPending)
    {
        return true;
    }

    for (auto& executor : _componentTransitionExecutors)
    {
        if (executor._isPending)
        {
            return false;
        }
    }
    _isTransitionPending = false;
    Logger::debug(
        LIFECYCLE, "%s level %d done", getTransitionString(_transition), _transitionLevel);
    _componentTransitionExecutors.clear();
    if (_transition != ILifecycleComponent::Transition::Type::INIT)
    {
        auto const transition = _transition;
        _transition           = ILifecycleComponent::Transition::Type::INIT;
        LifecycleListenerListType::iterator it;
        {
            ::async::LockType const lock;
            it = _listeners.begin();
        }
        while (it != _listeners.end())
        {
            ILifecycleListener& listener = *it;
            {
                ::async::LockType const lock;
                ++it; // This is how an iterator is used
            }
            listener.lifecycleLevelReached(_currentLevel, transition);
        }
    }
    return true;
}

char const*
LifecycleManager::getTransitionString(ILifecycleComponent::Transition::Type const transition)
{
    switch (transition)
    {
        case ILifecycleComponent::Transition::Type::RUN:
        {
            return "Run";
        }
        case ILifecycleComponent::Transition::Type::SHUTDOWN:
        {
            return "Shutdown";
        }
        // case ILifecycleComponent::Transition::INIT:
        default:
        {
            return "Initialize";
        }
    }
}

void LifecycleManager::ComponentTransitionExecutor::execute()
{
    _component.startTransition(_transition);
}

} // namespace lifecycle
