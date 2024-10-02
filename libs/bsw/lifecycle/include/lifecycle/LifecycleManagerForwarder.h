// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_B5CFAF50_8C23_4D08_9A83_02DBA72C668E
#define GUARD_B5CFAF50_8C23_4D08_9A83_02DBA72C668E

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

#endif // GUARD_B5CFAF50_8C23_4D08_9A83_02DBA72C668E
