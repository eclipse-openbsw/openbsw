// Copyright 2024 Accenture.

#include "lifecycle/LifecycleComponent.h"

#include "lifecycle/LifecycleComponentCallbackMock.h"

namespace
{
using namespace ::lifecycle;
using namespace ::testing;

class LifecycleComponentMock : public LifecycleComponent
{
public:
    MOCK_METHOD1(getTransitionContext, ::async::ContextType(Transition::Type transition));
    MOCK_METHOD0(init, void());
    MOCK_METHOD0(run, void());
    MOCK_METHOD0(shutdown, void());

    using LifecycleComponent::transitionDone;
};

struct LifecycleComponentTest : Test
{
    StrictMock<LifecycleComponentCallbackMock> _callbackMock;
};

TEST_F(LifecycleComponentTest, testCompleteComponent)
{
    StrictMock<LifecycleComponentMock> cut;

    cut.initCallback(_callbackMock);

    {
        // expect init
        EXPECT_CALL(cut, init());
        cut.startTransition(ILifecycleComponent::Transition::Type::INIT);
        Mock::VerifyAndClearExpectations(&cut);
        EXPECT_CALL(_callbackMock, transitionDone(Ref(cut)));
        cut.transitionDone();
        Mock::VerifyAndClearExpectations(&_callbackMock);
    }

    {
        // expect run
        EXPECT_CALL(cut, run());
        cut.startTransition(ILifecycleComponent::Transition::Type::RUN);
        Mock::VerifyAndClearExpectations(&cut);

        EXPECT_CALL(_callbackMock, transitionDone(Ref(cut)));
        cut.transitionDone();
        Mock::VerifyAndClearExpectations(&_callbackMock);
    }

    {
        // expect shutdown
        EXPECT_CALL(cut, shutdown());
        cut.startTransition(ILifecycleComponent::Transition::Type::SHUTDOWN);
        Mock::VerifyAndClearExpectations(&cut);

        EXPECT_CALL(_callbackMock, transitionDone(Ref(cut)));
        cut.transitionDone();
        Mock::VerifyAndClearExpectations(&_callbackMock);
    }
}

} // anonymous namespace
