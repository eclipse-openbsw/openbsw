// Copyright 2025 Accenture.

#include "doip/server/DoIpServerVehicleIdentification.h"

#include "doip/common/DoIpConstants.h"
#include "doip/server/DoIpServerVehicleIdentificationCallbackMock.h"

#include <util/estd/gtest_extensions.h>

#include <estd/array.h>
#include <estd/memory.h>

using namespace ::testing;
using namespace ::doip;
using ::estd::test::Slice;

namespace
{
struct DoIpServerVehicleIdentificationTest : Test
{
    DoIpServerVehicleIdentificationTest()
    {
        ON_CALL(fVehicleIdentificationCallbackMock, getPowerMode())
            .WillByDefault(Return(DoIpConstants::DiagnosticPowerMode::READY));
    }

    doip::DoIpServerVehicleIdentificationCallbackMock fVehicleIdentificationCallbackMock;
};

TEST_F(DoIpServerVehicleIdentificationTest, CallsFunctions)
{
    DoIpServerVehicleIdentification cut(
        DoIpServerVehicleIdentification::GetVinCallback::create<
            DoIpServerVehicleIdentificationCallbackMock,
            &DoIpServerVehicleIdentificationCallbackMock::getVin>(
            fVehicleIdentificationCallbackMock),
        DoIpServerVehicleIdentification::GetGidCallback::create<
            DoIpServerVehicleIdentificationCallbackMock,
            &DoIpServerVehicleIdentificationCallbackMock::getGid>(
            fVehicleIdentificationCallbackMock),
        DoIpServerVehicleIdentification::GetEidCallback::create<
            DoIpServerVehicleIdentificationCallbackMock,
            &DoIpServerVehicleIdentificationCallbackMock::getEid>(
            fVehicleIdentificationCallbackMock),
        DoIpServerVehicleIdentification::GetPowerModeCallback::create<
            DoIpServerVehicleIdentificationCallbackMock,
            &DoIpServerVehicleIdentificationCallbackMock::getPowerMode>(
            fVehicleIdentificationCallbackMock),
        DoIpServerVehicleIdentification::OnVirReceivedCallback::create<
            DoIpServerVehicleIdentificationCallbackMock,
            &DoIpServerVehicleIdentificationCallbackMock::onVirReceived>(
            fVehicleIdentificationCallbackMock),
        nullptr);

    {
        ::estd::array<char, 17> receivedVin{};
        EXPECT_CALL(fVehicleIdentificationCallbackMock, getVin(Slice(receivedVin.data(), 17U)));
        cut.getVin(receivedVin);
        Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    }
    {
        ::estd::array<uint8_t, 6> receivedGid{};
        EXPECT_CALL(fVehicleIdentificationCallbackMock, getGid(Slice(receivedGid.data(), 6U)));
        cut.getGid(receivedGid);
        Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    }
    {
        ::estd::array<uint8_t, 6> receivedEid{};
        EXPECT_CALL(fVehicleIdentificationCallbackMock, getEid(Slice(receivedEid.data(), 6U)));
        cut.getEid(receivedEid);
        Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    }
    {
        EXPECT_CALL(fVehicleIdentificationCallbackMock, getPowerMode());
        EXPECT_EQ(
            DoIpConstants::DiagnosticPowerMode(DoIpConstants::DiagnosticPowerMode::READY),
            cut.getPowerMode());
        Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    }
    {
        EXPECT_CALL(fVehicleIdentificationCallbackMock, onVirReceived());
        cut.onVirReceived();
        Mock::VerifyAndClearExpectations(&fVehicleIdentificationCallbackMock);
    }
}

} // namespace
