// Copyright 2024 Accenture.

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
