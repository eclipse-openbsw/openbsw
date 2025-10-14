// Copyright 2024 Accenture.

#include "uds/jobs/DataIdentifierJob.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

struct DataIdentifierJobTest : ::testing::Test
{
    DataIdentifierJob dataIdentifierReadRoutingTable{IMPLEMENTED_REQUEST_READ_DATAIDENTIFIER};
    DataIdentifierJob dataIdentifierWriteIPConfig{
        IMPLEMENTED_REQUEST_WRITE_DATAIDENTIFIER, REQUEST_LENGTH, AbstractDiagJob::EMPTY_RESPONSE};

    static uint8_t const IMPLEMENTED_REQUEST_READ_DATAIDENTIFIER[3];
    static uint8_t const IMPLEMENTED_REQUEST_WRITE_DATAIDENTIFIER[3];

    static uint16_t const REQUEST_LENGTH = 13;
};

uint8_t const DataIdentifierJobTest::IMPLEMENTED_REQUEST_READ_DATAIDENTIFIER[] = {0x22, 0x25, 0x09};

uint8_t const DataIdentifierJobTest::IMPLEMENTED_REQUEST_WRITE_DATAIDENTIFIER[]
    = {0x2E, 0x17, 0x2A};

TEST_F(
    DataIdentifierJobTest,
    verifyJobs_expect_implementedRequest_is_responsible_for_incoming_dataIdentifier)
{
    uint8_t const requestDataIdentifier1[] = {0x25, 0x09};

    uint8_t const requestDataIdentifier2[] = {0x17, 0x2A};

    EXPECT_EQ(
        DiagReturnCode::OK,
        dataIdentifierReadRoutingTable.verify(
            requestDataIdentifier1, sizeof(requestDataIdentifier1)));
    EXPECT_EQ(
        DiagReturnCode::OK,
        dataIdentifierWriteIPConfig.verify(requestDataIdentifier2, sizeof(requestDataIdentifier2)));
}

TEST_F(
    DataIdentifierJobTest,
    verifyJobs_expect_implementedRequest_is_not_responsible_for_incoming_dataIdentifier)
{
    uint8_t const requestDataIdentifier1[] = {0x25, 0x08};

    uint8_t const requestDataIdentifier2[] = {0x18, 0x2A};

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        dataIdentifierReadRoutingTable.verify(
            requestDataIdentifier1, sizeof(requestDataIdentifier1)));
    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        dataIdentifierWriteIPConfig.verify(requestDataIdentifier2, sizeof(requestDataIdentifier2)));
}

} // anonymous namespace
