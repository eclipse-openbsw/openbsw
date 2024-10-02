// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_24A1263C_CA16_48E8_9B49_83D558DC34BA
#define GUARD_24A1263C_CA16_48E8_9B49_83D558DC34BA

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

#endif // GUARD_24A1263C_CA16_48E8_9B49_83D558DC34BA
