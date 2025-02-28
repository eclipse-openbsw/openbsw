// Copyright 2024 Accenture.

#include "can/canframes/CANFrame.h"

#include "can/canframes/CanId.h"

#include <estd/assert.h>

#include <gmock/gmock.h>

using namespace can;
using namespace ::testing;

/**
 * \desc
 * Test verifying the default constructor of CANFrame
 */
TEST(CANFrameTest, DefaultConstructor)
{
    CANFrame frame;
    ASSERT_EQ(static_cast<uint16_t>(0x00), frame.getId());
    ASSERT_EQ(static_cast<uint8_t>(0), frame.getPayloadLength());
}

/**
 * \desc
 * Test verifying the init constructor of CANFrame
 */
TEST(CANFrameTest, InitConstructor)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    // constructor with too long payload
    {
        uint8_t payload[CANFrame::MAX_FRAME_LENGTH + 1];
        ASSERT_THROW(
            { CANFrame frame(0x345, payload, sizeof(payload)); }, ::estd::assert_exception);
    }

    // extended constructor with too long payload
    {
        uint8_t payload[CANFrame::MAX_FRAME_LENGTH + 1];
        ASSERT_THROW(
            { CANFrame frame(0x345, payload, sizeof(payload), true); }, ::estd::assert_exception);
    }

    // extended constructor with invalid extended id
    {
        uint8_t payload[CANFrame::MAX_FRAME_LENGTH];
        ASSERT_THROW(
            { CANFrame frame(0xffffffffU, payload, sizeof(payload), true); },
            ::estd::assert_exception);
    }
}

/**
 * \desc
 * Test to verify comparison operator of CANFrame
 */
TEST(CANFrameTest, Equality)
{
    CANFrame frame1, frame2;
    ASSERT_TRUE(frame1 == frame2);

    // test different ids
    frame1.setId(0x1);
    ASSERT_TRUE(!(frame1 == frame2));

    frame1.setId(0x100);
    frame2.setId(0x100);
    uint8_t p1[8];
    uint8_t p2[8];
    for (uint8_t i = 1; i <= CANFrame::MAX_FRAME_LENGTH; ++i)
    { // test all valid frame length values
        frame1.setPayload(p1, i);
        frame2.setPayload(p2, i);
        for (uint8_t j = 0; j < i; ++j)
        { // init payload
            frame1.getPayload()[j] = j;
            frame2.getPayload()[j] = j;
        }
        ASSERT_TRUE(frame1 == frame2);
    }

    // test same
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
    frame1 = frame1;
#pragma GCC diagnostic pop

    // test different payload length values
    uint8_t payload[CANFrame::MAX_FRAME_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};
    frame1.setId(0x100);
    frame2.setId(0x100);
    frame1.setPayload(payload, CANFrame::MAX_FRAME_LENGTH);
    frame2.setPayload(payload, CANFrame::MAX_FRAME_LENGTH);
    ASSERT_TRUE(frame1 == frame2);
    frame1.setPayloadLength(4);
    ASSERT_TRUE(!(frame1 == frame2));

    // test different payloads
    uint8_t payload2[CANFrame::MAX_FRAME_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};
    frame1.setId(0x100);
    frame2.setId(0x100);
    frame1.setPayload(payload, CANFrame::MAX_FRAME_LENGTH);
    frame2.setPayload(payload2, CANFrame::MAX_FRAME_LENGTH);
    ASSERT_TRUE(frame1 == frame2);
    ASSERT_EQ(frame1, frame2);
    frame1.getPayload()[0] = 1;
    ASSERT_TRUE(!(frame1 == frame2));
    frame1.getPayload()[0] = 0;
    frame2.setId(0x17000020);
    ASSERT_TRUE(!(frame1 == frame2));
    frame2.setId(0x100);
    ASSERT_TRUE(frame1 == frame2);
    frame1.setPayloadLength(4);
    frame1.setPayloadLength(CANFrame::MAX_FRAME_LENGTH);
    frame1.setPayloadLength(4);
    frame1.setPayloadLength(CANFrame::MAX_FRAME_LENGTH + 1);
}

/**
 * \desc
 * Verifies setter methods of CANFrame
 */
TEST(CANFrameTest, Setter)
{
    CANFrame frame;
    uint16_t id = 0x555;
    uint8_t payload[CANFrame::MAX_FRAME_LENGTH];
    for (uint8_t i = 0; i < CANFrame::MAX_FRAME_LENGTH; ++i)
    { // init payload
        payload[i] = i;
    }
    frame.setId(id);
    ASSERT_EQ(id, frame.getId());
    uint32_t extId = CanId::Extended<0x17002345>::value;
    frame.setId(extId);
    ASSERT_EQ(extId, frame.getId());

    ASSERT_EQ(CANFrame::MAX_FRAME_LENGTH, frame.getMaxPayloadLength());

    frame.setPayload(payload, CANFrame::MAX_FRAME_LENGTH);
    ASSERT_EQ(CANFrame::MAX_FRAME_LENGTH, frame.getPayloadLength());

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ASSERT_THROW(
        { frame.setPayload(payload, CANFrame::MAX_FRAME_LENGTH + 1); }, ::estd::assert_exception);

    for (uint8_t i = 0; i <= CANFrame::MAX_FRAME_LENGTH; ++i)
    {
        frame.setPayload(payload, i);
        ASSERT_EQ(static_cast<uint8_t>(i), frame.getPayloadLength());
    }

    for (uint8_t i = 0; i <= CANFrame::MAX_FRAME_LENGTH; ++i)
    {
        frame.setPayloadLength(i);
        ASSERT_EQ(static_cast<uint8_t>(i), frame.getPayloadLength());
    }

    uint32_t timestamp = 0x23428242;
    frame.setTimestamp(timestamp);
    ASSERT_EQ(timestamp, frame.timestamp());
}

/**
 * \desc
 * Verification of CANFrames assignment operator
 */
TEST(CANFrameTest, Assignment)
{
    uint8_t payload1[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t payload2[CANFrame::MAX_FRAME_LENGTH];
    CANFrame frame1(0x0, payload1, 4, false);
    CANFrame frame2(0x0, payload2, CANFrame::MAX_FRAME_LENGTH, false);

    ASSERT_TRUE(!(frame1 == frame2));
    frame2 = frame1;
    ASSERT_TRUE(frame1 == frame2);
    ASSERT_EQ(frame1.getId(), frame2.getId());
    ASSERT_EQ(frame1.getPayloadLength(), frame2.getPayloadLength());

    for (uint8_t i = 0; i < 4; ++i)
    {
        ASSERT_EQ(frame1.getPayload()[i], frame2.getPayload()[i]);
    }
}

/**
 * \desc
 * Verify that assertion is thrown if a too large CANFrame is assigned to
 * a smaller one.
 */
TEST(CANFrameTest, IllegalAssignment)
{
    uint8_t payload1[4]{};
    uint8_t payload2[5]{};
    CANFrame frame1(0x0, payload1, 4, false);
    CANFrame frame2(0x0, payload2, 5, false);
    //    ASSERT_THROW(
    //        { frame1 = frame2; },
    //        AssertException
    //    );
}
