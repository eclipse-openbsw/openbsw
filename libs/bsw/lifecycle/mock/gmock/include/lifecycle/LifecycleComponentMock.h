// Copyright 2024 Accenture.

/**
 * \ingroup lifecycle
 */
#ifndef GUARD_D1AD1771_8694_4A52_BA38_AA95B1CCC135
#define GUARD_D1AD1771_8694_4A52_BA38_AA95B1CCC135

#include "lifecycle/ILifecycleComponent.h"

#include <gmock/gmock.h>

namespace lifecycle
{
class LifecycleComponentMock : public ILifecycleComponent
{
public:
    MOCK_METHOD1(initCallback, void(ILifecycleComponentCallback& callback));
    MOCK_METHOD1(getTransitionContext, ::async::ContextType(Transition::Type transition));
    MOCK_METHOD1(startTransition, void(Transition::Type transition));
};

} // namespace lifecycle

#endif // GUARD_D1AD1771_8694_4A52_BA38_AA95B1CCC135
