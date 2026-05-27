/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleComponent.h"

#include <etl/intrusive_forward_list.h>

namespace lifecycle
{
class ILifecycleListener : public ::etl::forward_link<0>
{
public:
    virtual void
    lifecycleLevelReached(uint8_t level, ILifecycleComponent::Transition::Type transition)
        = 0;
    virtual ~ILifecycleListener() = default;
};

} // namespace lifecycle
