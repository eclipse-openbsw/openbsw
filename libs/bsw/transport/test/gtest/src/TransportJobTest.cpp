// Copyright 2024 Accenture.

#include "transport/TransportJob.h"

#include "transport/ITransportMessageProcessedListener.h"
#include "transport/TransportMessage.h"

#include <estd/assert.h>
#include <platform/estdint.h>

#include <gmock/gmock.h>

#include <cstdlib>

using namespace transport;

using namespace ::testing;

namespace
{
/**
 * Tests for TransportJob
 */
class TransportJobTest
: public ::testing::Test
, public transport::ITransportMessageProcessedListener
{
public:
    static uint8_t const BUFFER_LENGTH = 16; // Max. payload length == 8

    void SetUp() override
    {
        fBuffer = new uint8_t[BUFFER_LENGTH];
        m.init(fBuffer, BUFFER_LENGTH);
    }

    void TearDown() override { delete[] fBuffer; }

    void transportMessageProcessed(
        transport::TransportMessage& /* transportMessage */, ProcessingResult /* result */) override
    {}

protected:
    TransportMessage m;
    uint8_t* fBuffer;
};

uint8_t const TransportJobTest::BUFFER_LENGTH;

TEST_F(TransportJobTest, TestConstructor)
{
    transport::TransportJob job;
    ASSERT_EQ(nullptr, job.getTransportMessage());
    ASSERT_EQ(nullptr, job.getProcessedListener());

    transport::TransportJob job2(m, nullptr);
    ASSERT_EQ(&m, job2.getTransportMessage());
    ASSERT_EQ(nullptr, job2.getProcessedListener());
}

TEST_F(TransportJobTest, TestSetTransportMessage)
{
    transport::TransportJob job;
    job.setTransportMessage(m);
    ASSERT_EQ(&m, job.getTransportMessage());
    ASSERT_EQ(nullptr, job.getProcessedListener());
}

TEST_F(TransportJobTest, TestSetTransportMessageProcessedListener)
{
    transport::TransportJob job;
    job.setProcessedListener(this);
    ASSERT_EQ(nullptr, job.getTransportMessage());
    ASSERT_EQ(this, job.getProcessedListener());
    job.setProcessedListener(nullptr);
    ASSERT_EQ(nullptr, job.getProcessedListener());
}

TEST_F(TransportJobTest, TestAssignmentOperator)
{
    transport::TransportJob job;
    job.setTransportMessage(m);
    job.setProcessedListener(this);
    ASSERT_EQ(&m, job.getTransportMessage());
    ASSERT_EQ(this, job.getProcessedListener());
    transport::TransportJob job2;
    job = job2;
    ASSERT_EQ(nullptr, job.getTransportMessage());
    ASSERT_EQ(nullptr, job.getProcessedListener());
}

} // anonymous namespace
