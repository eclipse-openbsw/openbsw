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

#include <async/Async.h>

namespace lifecycle
{
class ILifecycleComponent;

class ILifecycleComponentCallback
{
public:
    virtual void transitionDone(ILifecycleComponent& component) = 0;

    virtual ~ILifecycleComponentCallback() = default;

private:
    ILifecycleComponentCallback& operator=(ILifecycleComponentCallback const&) = delete;
};

} // namespace lifecycle
