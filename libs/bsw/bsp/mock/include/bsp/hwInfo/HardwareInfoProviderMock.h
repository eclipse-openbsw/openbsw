// Copyright 2024 Accenture.

#ifndef GUARD_76305CEA_D06B_468A_A68F_F0AB4FEF9E5B
#define GUARD_76305CEA_D06B_468A_A68F_F0AB4FEF9E5B

#include "bsp/hwInfo/IHardwareInfoProvider.h"

#include <gmock/gmock.h>

namespace bsp
{
class HardwareInfoProviderMock : public IHardwareInfoProvider
{
public:
    MOCK_METHOD0(isFlexrayHighVersion, bool());
    MOCK_METHOD0(isMostAvailable, bool());
    MOCK_METHOD0(isComBoard, bool());
    MOCK_METHOD0(isDebugCanAvailable, bool());
};

} // namespace bsp

#endif /* GUARD_76305CEA_D06B_468A_A68F_F0AB4FEF9E5B */
