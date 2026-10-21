/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "features/communication/samplePduWithCrcCommon.h"
#include "features/communication/samplePduWithCrcReader.h"
#include "features/communication/samplePduWithCrcReaderMock.h"
#include "middleware/signal/qualifier.h"

namespace features::communication::test
{

using ::testing::NiceMock;
using Qualifier = middleware::signal::integration::Qualifier;

TEST(PduReaderMockTest, ReaderCallsForwardToGMockInterface)
{
    // Arrange
    samplePduWithCrc const expected{0xA5U, 0x12U, 0x3456U, 0x789ABCDEU};

    NiceMock<samplePduWithCrcReaderMock> readerMock{};
    NiceMock<Qualifier> qualifierMock{};
    samplePduWithCrcReader reader{&qualifierMock};

    readerMock.ON_CALL_READ_RETURNS(expected);
    qualifierMock.ON_CALL_GET_QUALIFICATION(Qualifier::Qualification::Ok);

    readerMock.EXPECT_READ();

    // Act
    samplePduWithCrc const actual = reader.read();

    // Assert
    EXPECT_EQ(actual.crc, expected.crc);
    EXPECT_EQ(actual.firstSignal, expected.firstSignal);
    EXPECT_EQ(actual.secondSignal, expected.secondSignal);
    EXPECT_EQ(actual.thirdSignal, expected.thirdSignal);
}

} // namespace features::communication::test
