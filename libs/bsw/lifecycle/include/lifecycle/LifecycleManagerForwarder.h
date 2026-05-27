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

#include "lifecycle/ILifecycleManager.h"

namespace lifecycle
{

class LifecycleManagerForwarder : public ILifecycleManager
{
public:
    void init(ILifecycleManager& lifecycleManager);

    uint8_t getLevelCount() const override;
    void transitionToLevel(uint8_t level) override;
    void addLifecycleListener(ILifecycleListener& listener) override;
    void removeLifecycleListener(ILifecycleListener& listener) override;

private:
    ILifecycleManager* _lifecycleManager = nullptr;
};

} // namespace lifecycle
