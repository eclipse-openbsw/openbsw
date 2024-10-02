// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_8A55E0D9_E4FF_4108_BDC9_6C1A0973E983
#define GUARD_8A55E0D9_E4FF_4108_BDC9_6C1A0973E983

#include "lifecycle/ILifecycleManager.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleManagerMock : public ILifecycleManager
{
public:
    MOCK_CONST_METHOD0(getLevelCount, uint8_t());
    MOCK_METHOD1(transitionToLevel, void(uint8_t level));
    MOCK_METHOD1(addLifecycleListener, void(ILifecycleListener& listener));
    MOCK_METHOD1(removeLifecycleListener, void(ILifecycleListener& listener));
};

} // namespace lifecycle

#endif // GUARD_8A55E0D9_E4FF_4108_BDC9_6C1A0973E983
