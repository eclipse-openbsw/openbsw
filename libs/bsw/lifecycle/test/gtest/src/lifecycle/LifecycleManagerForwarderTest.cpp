// Copyright 2024 Accenture.

#include "lifecycle/LifecycleManagerForwarder.h"

#include "lifecycle/LifecycleListenerMock.h"
#include "lifecycle/LifecycleManagerMock.h"

#include <gtest/esr_extensions.h>

namespace
{
using namespace ::lifecycle;
using namespace ::testing;

TEST(LifecycleManagerForwarderTest, testForwarding)
{
    LifecycleManagerForwarder cut;
    StrictMock<LifecycleManagerMock> lifecycleManagerMock;
    StrictMock<LifecycleListenerMock> lifecycleListenerMock;
    {
        cut.init(lifecycleManagerMock);
    }
    {
        EXPECT_CALL(lifecycleManagerMock, getLevelCount()).WillOnce(Return(17U));
        EXPECT_EQ(17U, cut.getLevelCount());
        Mock::VerifyAndClearExpectations(&lifecycleManagerMock);
    }
    {
        EXPECT_CALL(lifecycleManagerMock, transitionToLevel(7U));
        cut.transitionToLevel(7U);
        Mock::VerifyAndClearExpectations(&lifecycleManagerMock);
    }
    {
        EXPECT_CALL(lifecycleManagerMock, addLifecycleListener(Ref(lifecycleListenerMock)));
        cut.addLifecycleListener(lifecycleListenerMock);
        Mock::VerifyAndClearExpectations(&lifecycleManagerMock);
    }
    {
        EXPECT_CALL(lifecycleManagerMock, removeLifecycleListener(Ref(lifecycleListenerMock)));
        cut.removeLifecycleListener(lifecycleListenerMock);
        Mock::VerifyAndClearExpectations(&lifecycleManagerMock);
    }
}

} // anonymous namespace
