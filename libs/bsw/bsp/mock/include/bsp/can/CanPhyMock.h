// Copyright 2024 Accenture.

#ifndef GUARD_C477BFD1_B07B_4651_9E17_C0EE83C24524
#define GUARD_C477BFD1_B07B_4651_9E17_C0EE83C24524

#include "bsp/can/ICanPhy.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace can
{
class CanPhyMock : public ICanPhy
{
public:
    MOCK_METHOD2(setOperatingMode, ::bsp::BspReturnCode(OperatingMode, ChannelMask));
    MOCK_CONST_METHOD1(operatingMode, OperatingMode(uint8_t));
    MOCK_CONST_METHOD1(hasError, bool(uint8_t));
    MOCK_CONST_METHOD0(numberOfChannels, uint8_t());
};

} // namespace can
} // namespace bsp

#endif /* GUARD_C477BFD1_B07B_4651_9E17_C0EE83C24524 */
