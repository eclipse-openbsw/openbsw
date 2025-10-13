// Copyright 2024 Accenture.

#include "transport/TransportMessage.h"

#include "transport/DataProgressListenerMock.h"

#include <etl/span.h>

#include <gmock/gmock.h>

#include <cstdlib>

using namespace ::transport;
using namespace ::testing;

struct TransportMessageTest : Test
{
    static uint8_t const BUFFER_LENGTH = 16; // Max. payload length == 8

    TransportMessageTest() { m.init(fBuffer, BUFFER_LENGTH); }

    TransportMessage m;
    uint8_t fBuffer[BUFFER_LENGTH];
};

uint8_t const TransportMessageTest::BUFFER_LENGTH;

TEST_F(TransportMessageTest, TestAsserts)
{
    TransportMessage t;
    ASSERT_THROW(t.init(nullptr, 10), ::etl::exception);
    ASSERT_THROW(t.setServiceId(1), ::etl::exception);
    ASSERT_THROW(t.setPayloadLength(100), ::etl::exception);
}

TEST_F(TransportMessageTest, DefaultConstructor)
{
    TransportMessage fTxTpMessage;
    EXPECT_THAT(fTxTpMessage.getBuffer(), IsNull());
    EXPECT_EQ(0U, fTxTpMessage.getValidBytes());
}

TEST_F(TransportMessageTest, Constructor)
{
    uint8_t x;
    TransportMessage fTxTpMessage(&x, 1);
    EXPECT_EQ(&x, fTxTpMessage.getBuffer());
    EXPECT_EQ(0U, fTxTpMessage.getValidBytes());
}

TEST_F(TransportMessageTest, Init)
{
    m.init(fBuffer, BUFFER_LENGTH);
    EXPECT_EQ(fBuffer, m.getBuffer());
    EXPECT_EQ(BUFFER_LENGTH, m.getBufferLength());
    EXPECT_EQ(0U, m.getValidBytes());
    EXPECT_EQ(0U, m.getPayloadLength());
    // init with length 0
    m.init(fBuffer, 0);
    EXPECT_EQ(fBuffer, m.getBuffer());
    EXPECT_EQ(0U, m.getBufferLength());
    // empty init
    m.init(nullptr, 0);
    EXPECT_EQ(nullptr, m.getBuffer());
    EXPECT_EQ(0U, m.getBufferLength());
    ASSERT_THROW(m.init(nullptr, 1), ::etl::exception);
    EXPECT_EQ(0U, m.missingBytes());
}

TEST_F(TransportMessageTest, GetSetSourceId)
{
    m.setSourceAddress(2U);
    EXPECT_EQ(2U, m.getSourceId());

    m.setSourceAddress(0xFFFFU);
    EXPECT_EQ(0xFFFFU, m.getSourceId());
}

TEST_F(TransportMessageTest, GetSetTargetId)
{
    m.setTargetAddress(42U);
    EXPECT_EQ(42U, m.getTargetId());

    m.setTargetAddress(0xFFFFU);
    EXPECT_EQ(0xFFFFU, m.getTargetId());
}

TEST_F(TransportMessageTest, GetSetServiceId)
{
    m.setServiceId(0U);
    EXPECT_EQ(0U, m.getServiceId());

    m.setServiceId(0xFFU);
    EXPECT_EQ(0xFFU, m.getServiceId());
}

TEST_F(TransportMessageTest, GetPayload)
{
    EXPECT_EQ(fBuffer, m.getPayload());
    EXPECT_EQ(m.getPayload(), &m[0]);

    TransportMessage const& cm = m;
    EXPECT_EQ(m.getPayload(), cm.getPayload());
}

TEST_F(TransportMessageTest, GetResetValidBytes)
{
    uint16_t MESSAGE_SIZE         = BUFFER_LENGTH;
    uint8_t fpData[BUFFER_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_OK, m.append(fpData, MESSAGE_SIZE));
    EXPECT_EQ(MESSAGE_SIZE, m.getValidBytes());
    m.resetValidBytes();
    EXPECT_EQ(0U, m.getValidBytes());
}

TEST_F(TransportMessageTest, OperatorIndex)
{
    uint8_t fData[BUFFER_LENGTH] = {0};
    fData[0]                     = 10;
    m.init(fData, BUFFER_LENGTH);

    TransportMessage const& cm = m;
    EXPECT_EQ(m[0], cm[0]);
    EXPECT_EQ(cm[0], 10U);
}

TEST_F(TransportMessageTest, IsComplete)
{
    uint16_t MESSAGE_SIZE = BUFFER_LENGTH;
    m.setPayloadLength(MESSAGE_SIZE);
    EXPECT_EQ(MESSAGE_SIZE, m.getPayloadLength());

    for (uint16_t i = 0; i < MESSAGE_SIZE; i++)
    {
        EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_OK, m.increaseValidBytes(i));
        EXPECT_FALSE(m.isComplete());
        m.resetValidBytes();
    }
    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_OK, m.increaseValidBytes(MESSAGE_SIZE));
    EXPECT_TRUE(m.isComplete());
}

TEST_F(TransportMessageTest, GetMaxPayloadLength)
{
    EXPECT_EQ((uint16_t)(BUFFER_LENGTH), m.getMaxPayloadLength());
}

TEST_F(TransportMessageTest, IncreaseValidBytes)
{
    m.increaseValidBytes(1);
    EXPECT_EQ((uint16_t)1, m.getValidBytes());
}

TEST_F(TransportMessageTest, IncreaseValidBytesAssertion)
{
    TransportMessage::ErrorCode result;
    result = m.increaseValidBytes(m.getMaxPayloadLength());
    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_OK, result);
    EXPECT_EQ(m.getMaxPayloadLength(), m.getValidBytes());

    result = m.increaseValidBytes(1);
    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_LENGTH_EXCEEDED, result);
}

TEST_F(TransportMessageTest, SetGetPayloadLength)
{
    uint16_t const testPayloadLength = m.getMaxPayloadLength();
    m.setPayloadLength(testPayloadLength);
    EXPECT_EQ(testPayloadLength, m.getPayloadLength());
}

TEST_F(TransportMessageTest, SetPayloadLengthAssertion)
{
    uint16_t testPayloadLength = m.getMaxPayloadLength() + 1;
    ASSERT_THROW(m.setPayloadLength(testPayloadLength), ::etl::exception);
}

TEST_F(TransportMessageTest, TransportMessageAppend)
{
    uint8_t buffer[16] = {0};
    TransportMessage m;
    m.init(buffer, 16);
    m.setTargetAddress(0x1);
    m.setSourceAddress(0x2);
    uint8_t const data[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_LENGTH_EXCEEDED, m.append(data, 17));

    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_OK, m.append(data, 8));
    EXPECT_EQ(8U, m.getValidBytes());

    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_OK, m.append(data, 8));
    EXPECT_EQ(16U, m.getValidBytes());

    EXPECT_THAT(::etl::span<uint8_t>(buffer).first(8), ElementsAreArray(data));
    EXPECT_THAT(::etl::span<uint8_t>(buffer).subspan(8), ElementsAreArray(data));

    EXPECT_EQ(0x1U, m.getTargetId());
    EXPECT_EQ(0x2U, m.getSourceId());
    EXPECT_EQ(TransportMessage::ErrorCode::TP_MSG_LENGTH_EXCEEDED, m.append(0x00));
}

TEST_F(TransportMessageTest, CompareOperator)
{
    uint16_t const MESSAGE_SIZE = BUFFER_LENGTH;

    TransportMessage localMessage;
    uint8_t fpLocalBuffer[BUFFER_LENGTH];

    // let the buffer have same contents
    memset(fpLocalBuffer, 0xAB, BUFFER_LENGTH);
    memset(fBuffer, 0xAB, BUFFER_LENGTH);

    localMessage.init(fpLocalBuffer, BUFFER_LENGTH);
    localMessage.setSourceAddress(0xC0);
    localMessage.setTargetAddress(0xC1);
    localMessage.setPayloadLength(BUFFER_LENGTH);
    m.init(fBuffer, BUFFER_LENGTH);
    m.setSourceAddress(0xC0);
    m.setTargetAddress(0xC1);
    m.setPayloadLength(BUFFER_LENGTH);
    EXPECT_EQ(true, m == localMessage);

    // payloadLength different
    m.setPayloadLength(MESSAGE_SIZE);
    EXPECT_EQ(MESSAGE_SIZE, m.getPayloadLength());
    localMessage.setPayloadLength(MESSAGE_SIZE - 1);
    EXPECT_EQ((uint16_t)(MESSAGE_SIZE - 1), localMessage.getPayloadLength());
    EXPECT_FALSE(m == localMessage);

    // restore equality
    m.setPayloadLength(MESSAGE_SIZE);
    EXPECT_EQ(MESSAGE_SIZE, m.getPayloadLength());
    localMessage.setPayloadLength(MESSAGE_SIZE);
    EXPECT_EQ((uint16_t)(MESSAGE_SIZE), localMessage.getPayloadLength());
    EXPECT_EQ(m, localMessage);

    // number of valid bytes different
    m.resetValidBytes();
    EXPECT_EQ(uint16_t(0), m.getValidBytes());
    localMessage.resetValidBytes();
    EXPECT_EQ(uint16_t(0), localMessage.getValidBytes());
    EXPECT_EQ(m, localMessage);
    m.increaseValidBytes(MESSAGE_SIZE - 1);
    localMessage.increaseValidBytes(MESSAGE_SIZE);
    EXPECT_FALSE(m == localMessage);

    // target different
    m.resetValidBytes();
    localMessage.resetValidBytes();
    EXPECT_EQ(m, localMessage);
    EXPECT_EQ(m.getTargetId(), localMessage.getTargetId());
    m.setTargetAddress(0xF1);
    localMessage.setTargetAddress(0xF2);
    EXPECT_FALSE(m == localMessage);

    // source different
    m.setTargetAddress(0xF1);
    localMessage.setTargetAddress(0xF1);
    EXPECT_EQ(m, localMessage);
    m.setSourceAddress(0xAB);
    localMessage.setSourceAddress(0xCD);
    EXPECT_FALSE(m == localMessage);

    // restore equality
    m.setSourceAddress(0xAB);
    localMessage.setSourceAddress(0xAB);
    EXPECT_EQ(m, localMessage);

    // payload contents different
    for (int i = 0; i < MESSAGE_SIZE; i++)
    {
        m.append(i % MESSAGE_SIZE);
        localMessage.append((i % MESSAGE_SIZE) + 1);
    }
    EXPECT_FALSE(m == localMessage);
}

TEST_F(TransportMessageTest, CompareSameEverything)
{
    uint16_t MESSAGE_SIZE = BUFFER_LENGTH;

    TransportMessage localMessage;
    uint8_t fpLocalBuffer[BUFFER_LENGTH];

    // let the buffer have same contents
    memset(fpLocalBuffer, 0xAB, BUFFER_LENGTH);
    memset(fBuffer, 0xAB, BUFFER_LENGTH);

    localMessage.init(fpLocalBuffer, BUFFER_LENGTH);
    m.init(fBuffer, BUFFER_LENGTH);

    m.setSourceAddress(0xC0);
    m.setTargetAddress(0xC1);
    localMessage.setSourceAddress(0xC0);
    localMessage.setTargetAddress(0xC1);
    EXPECT_EQ(m, localMessage);

    // payload contents different
    for (int i = 0; i < MESSAGE_SIZE; i++)
    {
        m.append(i % MESSAGE_SIZE);
        localMessage.append((i % MESSAGE_SIZE));
    }
    EXPECT_EQ(m, localMessage);
}

TEST_F(TransportMessageTest, DataProgressListenerNoListener)
{
    StrictMock<DataProgressListenerMock> listener;
    EXPECT_CALL(listener, dataProgressed(_, _)).Times(0);
    m.increaseValidBytes(1);
}

TEST_F(TransportMessageTest, DataProgressListenerListener)
{
    StrictMock<DataProgressListenerMock> listener;
    EXPECT_FALSE(m.isDataProgressListener(listener));
    m.setDataProgressListener(listener);
    EXPECT_TRUE(m.isDataProgressListener(listener));

    EXPECT_CALL(listener, dataProgressed(_, Eq(1U))).Times(1);
    m.increaseValidBytes(1);

    m.removeDataProgressListener();
    EXPECT_FALSE(m.isDataProgressListener(listener));
    EXPECT_CALL(listener, dataProgressed(_, _)).Times(0);
    m.increaseValidBytes(1);
}
