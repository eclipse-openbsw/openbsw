// Copyright 2024 Accenture.

#ifndef GUARD_82BE9184_1099_43C6_9986_902CF33F9224
#define GUARD_82BE9184_1099_43C6_9986_902CF33F9224

#include "bsp/power/AbstractPowerStateListener.h"
#include "bsp/power/IUnderVoltageListener.h"
#include "bsp/power/IVoltageManager.h"

#include <gmock/gmock.h>

namespace bsp
{
using namespace ::testing;

class VoltageManagerMock : public IVoltageManager
{
public:
    MOCK_METHOD2(
        addPowerStateListener, void(AbstractPowerStateListener& listener, uint16_t triggerVoltage));

    MOCK_METHOD1(removePowerStateListener, void(AbstractPowerStateListener& listener));

    MOCK_METHOD1(addUnderVoltageListener, void(IUnderVoltageListener& listener));

    MOCK_METHOD1(removeUnderVoltageListener, void(IUnderVoltageListener& listener));

    MOCK_METHOD0(getCurrentVoltage, uint16_t());

    MOCK_METHOD0(getUndervoltageOccurred, bool());
};

} // namespace bsp

#endif /* GUARD_82BE9184_1099_43C6_9986_902CF33F9224 */
