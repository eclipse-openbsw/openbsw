// Copyright 2024 Accenture.

#include "can/framemgmt/CANFrameListenerProxy.h"

#include "can/filter/IntervalFilter.h"
#include "can/framemgmt/CANFrameListenerMock.h"

namespace
{
using namespace can;
using namespace testing;

IntervalFilter filter(100U, 200U);

struct TestFilter
{
    static IFilter& getFilter() { return filter; }
};

TEST(CANFrameListenerProxyTest, TestAll)
{
    CANFrameListenerMock listenerMock;
    CANFrame frame(100U);
    {
        CANFrameListenerProxy<TestFilter> cut;
        cut.frameReceived(frame);
        cut.setCANFrameListener(listenerMock);
        EXPECT_CALL(listenerMock, frameReceived(Ref(frame)));
        cut.frameReceived(frame);
    }
    {
        CANFrameListenerProxy<TestFilter> cut(listenerMock);
        EXPECT_CALL(listenerMock, frameReceived(Ref(frame)));
        cut.frameReceived(frame);
        EXPECT_EQ(&filter, &cut.getFilter());
    }
}

} // namespace
