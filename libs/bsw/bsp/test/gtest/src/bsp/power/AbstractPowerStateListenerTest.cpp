// Copyright 2024 Accenture.

#include "bsp/power/AbstractPowerStateListenerMock.h"

#include <gmock/gmock.h>

namespace bsp
{
class VoltageManager
{
public:
    VoltageManager()
    {
        AbstractPowerStateListenerMock powerStateListener;
        EXPECT_EQ(0U, powerStateListener.getTriggerVoltage());
    }
};

} // namespace bsp

namespace
{
using namespace ::bsp;
using namespace testing;

TEST(AbstractPowerStateListener, SimpleTest) { VoltageManager voltageManager; }

} // anonymous namespace
