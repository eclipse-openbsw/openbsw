// Copyright 2024 Accenture.

#pragma once

#include "lifecycle/LifecycleManager.h"

#include <etl/array.h>
#include <etl/type_traits.h>
#include <etl/utility.h>

namespace lifecycle
{
namespace internal
{
struct LifecycleDesc
{
    char const* name;
    ILifecycleComponent& component;
};

template<size_t N>
struct LifecycleLevel
{
    static constexpr size_t NUM_COMPONENTS = N;
    static constexpr uint8_t LEVEL_WEIGHT  = 1U;

    ::etl::array<LifecycleDesc const, N> const components;
};

struct StoreLevelAction
{
    static constexpr size_t NUM_COMPONENTS = 0U;
    static constexpr uint8_t LEVEL_WEIGHT  = 0U;

    uint8_t& level;
};

struct EndTag
{};

template<typename T, typename... A>
constexpr size_t countDescriptors()
{
    return countDescriptors<T>() + countDescriptors<A...>();
}

template<>
constexpr size_t countDescriptors<LifecycleDesc>()
{
    return 1U;
}

template<>
constexpr size_t countDescriptors<EndTag>()
{
    return 0U;
}

template<typename T, typename... A>
constexpr size_t countComponents()
{
    return ::etl::remove_reference<T>::type::NUM_COMPONENTS + countComponents<A...>();
}

template<>
constexpr size_t countComponents<EndTag>()
{
    return 0U;
}

template<typename T, typename... A>
constexpr uint8_t countLevels()
{
    return ::etl::remove_reference<T>::type::LEVEL_WEIGHT + countLevels<A...>();
}

template<>
constexpr uint8_t countLevels<EndTag>()
{
    return 0U;
}

template<typename T>
constexpr T countMax(T const a, T const b)
{
    return (a < b) ? b : a;
}

template<typename T, typename... A>
constexpr size_t countComponentsPerLevel()
{
    return countMax(
        ::etl::remove_reference<T>::type::NUM_COMPONENTS, countComponentsPerLevel<A...>());
}

template<>
constexpr size_t countComponentsPerLevel<EndTag>()
{
    return 0U;
}

template<typename T>
inline void addLevel(uint8_t const levelIndex, LifecycleManager& manager, T&& level)
{
    for (auto const& desc : level.components)
    {
        manager.addComponent(desc.name, desc.component, levelIndex);
    }
}

template<>
inline void addLevel<StoreLevelAction>(
    uint8_t const levelIndex, LifecycleManager& manager, StoreLevelAction&& action)
{
    (void)manager;
    // stores currently defined level
    // we need to decrement incremented value here
    action.level = levelIndex - 1U;
}

template<>
inline void addLevel<EndTag>(uint8_t const levelIndex, LifecycleManager& manager, EndTag&& tag)
{
    (void)levelIndex;
    (void)manager;
    (void)tag;
}

template<typename T, typename... A>
inline void
initManager(uint8_t const currentLevelIndex, LifecycleManager& manager, T&& level, A&&... args)
{
    addLevel(currentLevelIndex, manager, ::etl::forward<T>(level));
    initManager(
        currentLevelIndex + ::etl::remove_reference<T>::type::LEVEL_WEIGHT,
        manager,
        ::etl::forward<A>(args)...);
}

template<>
inline void
initManager<EndTag>(uint8_t const currentLevelIndex, LifecycleManager& manager, EndTag&& tag)
{
    (void)currentLevelIndex;
    (void)manager;
    (void)tag;
}
} // namespace internal

inline constexpr internal::LifecycleDesc
makeComponent(char const* const name, ILifecycleComponent& component)
{
    return internal::LifecycleDesc{name, component};
}

template<typename... A>
inline constexpr internal::LifecycleLevel<internal::countDescriptors<A..., internal::EndTag>()>
makeLevel(A&&... args)
{
    return internal::LifecycleLevel<internal::countDescriptors<A..., internal::EndTag>()>{
        {{::etl::forward<A>(args)...}}};
}

inline constexpr internal::StoreLevelAction storeLevel(uint8_t& level)
{
    return internal::StoreLevelAction{level};
}

template<typename... A>
inline LifecycleManager& createLifecycleManager(
    ::async::ContextType const transitionContext,
    LifecycleManager::GetTimestampType const getTimestamp,
    A&&... args)
{
    using EndTag                           = internal::EndTag;
    constexpr size_t numComponents         = internal::countComponents<A..., EndTag>();
    constexpr size_t numComponentsPerLevel = internal::countComponentsPerLevel<A..., EndTag>();
    constexpr uint8_t numLevels            = internal::countLevels<A..., EndTag>();

    static declare::LifecycleManager<numComponents, numLevels, numComponentsPerLevel>
        createdManager(transitionContext, getTimestamp);
    internal::initManager(1U, createdManager, ::etl::forward<A>(args)..., EndTag());
    return createdManager;
}
} // namespace lifecycle
