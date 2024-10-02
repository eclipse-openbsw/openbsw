// Copyright 2024 Accenture.

#include "bsp/lin/linTransceiver/LinPhy.h"

#include <gmock/gmock.h>

namespace
{
class LinPhyMock : public ::bsp::lin::LinPhy
{
public:
    void setMode(Mode mode, uint32_t channel = 0) override
    {
        (void)channel;
        fMode = mode;
    }

    ErrorCode getPhyErrorStatus(uint32_t channel = 0) override
    {
        (void)channel;
        return ErrorCode();
    }
};

using namespace testing;

TEST(LinPhy, SimpleTest)
{
    LinPhyMock linPhy;
    linPhy.init();
    linPhy.setMode(::bsp::lin::ILinPhy::LIN_PHY_MODE_UNDEFINED);

    EXPECT_EQ(::bsp::lin::ILinPhy::LIN_PHY_MODE_UNDEFINED, linPhy.getMode());
}

} // anonymous namespace
