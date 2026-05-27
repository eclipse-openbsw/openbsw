/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/SingleContextLifecycleComponent.h"

namespace lifecycle
{
::async::ContextType
SingleContextLifecycleComponent::getTransitionContext(Transition::Type /*transition*/)
{
    return _context;
}

} // namespace lifecycle
