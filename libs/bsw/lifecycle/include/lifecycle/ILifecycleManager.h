// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

#include "lifecycle/ILifecycleListener.h"

#include <cstdint>

namespace lifecycle
{
class ILifecycleManager
{
public:
    virtual uint8_t getLevelCount() const                              = 0;
    virtual void transitionToLevel(uint8_t level)                      = 0;
    virtual void addLifecycleListener(ILifecycleListener& listener)    = 0;
    virtual void removeLifecycleListener(ILifecycleListener& listener) = 0;
    virtual ~ILifecycleManager()                                       = default;

private:
    ILifecycleManager& operator=(ILifecycleManager const&) = delete;
};

} // namespace lifecycle
