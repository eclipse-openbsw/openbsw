// Copyright 2024 Accenture.

#ifndef GUARD_DBB00E2B_553F_33B4_8FAC_70664F2F2C9F
#define GUARD_DBB00E2B_553F_33B4_8FAC_70664F2F2C9F

#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace lifecycle
{
class LifecycleManager;
}

namespace uds
{
class UdsLifecycleConnector : public IUdsLifecycleConnector
{
public:
    explicit UdsLifecycleConnector(lifecycle::LifecycleManager&) {}

    bool isModeChangePossible() const { return true; }

    bool requestPowerdown(bool rapid, uint8_t& time) { return true; }

    bool requestShutdown(ShutdownType type, uint32_t) { return true; }
};

} // namespace uds

#endif /*GUARD_DBB00E2B_553F_33B4_8FAC_70664F2F2C9F */
