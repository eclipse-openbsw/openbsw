// Copyright 2024 Accenture.

#ifndef GUARD_F2AE18B5_0932_406D_9CB0_23351EC60EFA
#define GUARD_F2AE18B5_0932_406D_9CB0_23351EC60EFA

#include "bsp/power/AbstractPowerStateListener.h"

#include <gmock/gmock.h>

namespace bsp
{
class AbstractPowerStateListenerMock : public AbstractPowerStateListener
{
public:
    MOCK_METHOD1(powerStateChanged, void(uint16_t));
};

} // namespace bsp

#endif /* GUARD_F2AE18B5_0932_406D_9CB0_23351EC60EFA */
