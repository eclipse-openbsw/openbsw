// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleComponent.h"

#include <estd/forward_list.h>

namespace lifecycle
{
class ILifecycleListener : public ::estd::forward_list_node<ILifecycleListener>
{
public:
    virtual void
    lifecycleLevelReached(uint8_t level, ILifecycleComponent::Transition::Type transition)
        = 0;
    virtual ~ILifecycleListener() = default;
};

} // namespace lifecycle

