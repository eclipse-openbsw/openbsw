// Copyright 2024 Accenture.

#include "uds/connection/PositiveResponse.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <util/estd/assert.h>

#include <gmock/gmock.h>

using namespace ::testing;

namespace
{
struct PositiveResponseTest : Test
{
    PositiveResponseTest() { response.reset(); }

    uds::IncomingDiagConnection conn{::async::CONTEXT_INVALID};
    uds::PositiveResponse response;
};

/**
 * \desc
 * A constructed PositiveResponse is not capable of storing any
 * data until init() has been called.
 */
TEST_F(PositiveResponseTest, Constructor)
{
    ASSERT_EQ(nullptr, response.getData());
    ASSERT_EQ(0, response.getLength());
    ASSERT_EQ(0, response.getMaximumLength());
    ASSERT_EQ(0, response.getAvailableDataLength());
    ASSERT_FALSE(response.appendUint8(uint8_t(0)));
    ASSERT_FALSE(response.appendUint16(uint16_t(0)));
    ASSERT_FALSE(response.appendUint24(uint32_t(0)));
    ASSERT_FALSE(response.appendUint32(uint32_t(0)));
    uint8_t data[10] = {0U};
    ASSERT_FALSE(response.appendData(data, sizeof(data)));
}

/**
 * \desc
 * A call to init() attaches a buffer to a PositiveResponse.
 */
TEST_F(PositiveResponseTest, Init)
{
    uint8_t buffer[4] = {0x01, 0x02, 0x03, 0x04};
    response.init(buffer, sizeof(buffer));
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    ASSERT_EQ(&buffer[0], response.getData());
    ASSERT_EQ(0, response.getLength());
    ASSERT_EQ(sizeof(buffer), response.getMaximumLength());
}

/**
 * \desc
 * Reset will reset the length of a PositiveResponse to zero.
 */
TEST_F(PositiveResponseTest, Reset)
{
    uint8_t buffer[4];
    response.init(buffer, sizeof(buffer));
    ASSERT_EQ(&buffer[0], response.getData());
    ASSERT_EQ(0, response.getLength());
    ASSERT_EQ(sizeof(buffer), response.getMaximumLength());

    ASSERT_TRUE(response.appendUint32(0x12345678));
    ASSERT_EQ(0x12, buffer[0]);
    ASSERT_EQ(0x34, buffer[1]);
    ASSERT_EQ(0x56, buffer[2]);
    ASSERT_EQ(0x78, buffer[3]);
    ASSERT_EQ(4, response.getLength());
    ASSERT_EQ(sizeof(buffer), response.getMaximumLength());

    response.reset();
    ASSERT_EQ(&buffer[0], response.getData());
    ASSERT_EQ(0, response.getLength());
    ASSERT_EQ(sizeof(buffer), response.getMaximumLength());
}

/**
 * \desc
 * This test verifies, that appendUint8() can append single bytes to a response until the
 * maximum number of bytes is reached.
 */
TEST_F(PositiveResponseTest, AppendUint8)
{
    uint8_t buffer[2];
    response.init(buffer, sizeof(buffer));
    ASSERT_TRUE(response.appendUint8(0x01));
    ASSERT_EQ(1, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_TRUE(response.appendUint8(0x02));
    ASSERT_EQ(2, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    // buffer is full, append must not work
    ASSERT_FALSE(response.appendUint8(0x02));
    ASSERT_EQ(2, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
}

/**
 * \desc
 * This test verifies, that appendUint16() can append uint16_t values to a response until the
 * maximum number of bytes is reached.
 */
TEST_F(PositiveResponseTest, AppendUint16)
{
    uint8_t buffer[4];
    response.init(buffer, sizeof(buffer));
    ASSERT_TRUE(response.appendUint16(0x0102));
    ASSERT_EQ(2, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_TRUE(response.appendUint16(0x0304));
    ASSERT_EQ(4, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    // buffer is full, append must not work
    ASSERT_FALSE(response.appendUint16(0x5555));
    ASSERT_EQ(4, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
}

/**
 * \desc
 * This test verifies, that appendUint24() can append uint32_t values (using 24 bits) to a response
 * until the maximum number of bytes is reached.
 */
TEST_F(PositiveResponseTest, AppendUint24)
{
    uint8_t buffer[6];
    response.init(buffer, sizeof(buffer));
    ASSERT_TRUE(response.appendUint24(0x010203));
    ASSERT_EQ(3, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_TRUE(response.appendUint24(0x040506));
    ASSERT_EQ(6, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    ASSERT_EQ(0x05, buffer[4]);
    ASSERT_EQ(0x06, buffer[5]);
    // buffer is full, append must not work
    ASSERT_FALSE(response.appendUint24(0x555555));
    ASSERT_EQ(6, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    ASSERT_EQ(0x05, buffer[4]);
    ASSERT_EQ(0x06, buffer[5]);
}

/**
 * \desc
 * This test verifies, that appendUint32() can append uint32_t values to a response until the
 * maximum number of bytes is reached.
 */
TEST_F(PositiveResponseTest, AppendUint32)
{
    uint8_t buffer[8];
    response.init(buffer, sizeof(buffer));
    ASSERT_TRUE(response.appendUint32(0x01020304));
    ASSERT_EQ(4, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    ASSERT_TRUE(response.appendUint32(0x05060708));
    ASSERT_EQ(8, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    ASSERT_EQ(0x05, buffer[4]);
    ASSERT_EQ(0x06, buffer[5]);
    ASSERT_EQ(0x07, buffer[6]);
    ASSERT_EQ(0x08, buffer[7]);
    // buffer is full, append must not work
    ASSERT_FALSE(response.appendUint32(0x55555555));
    ASSERT_EQ(8, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x02, buffer[1]);
    ASSERT_EQ(0x03, buffer[2]);
    ASSERT_EQ(0x04, buffer[3]);
    ASSERT_EQ(0x05, buffer[4]);
    ASSERT_EQ(0x06, buffer[5]);
    ASSERT_EQ(0x07, buffer[6]);
    ASSERT_EQ(0x08, buffer[7]);
}

/**
 * \desc
 * This test verifies, that calling appendData() can add bytes from a given array to a response.
 */
TEST_F(PositiveResponseTest, AppendData)
{
    uint8_t buffer[4];
    response.init(buffer, sizeof(buffer));
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    ASSERT_EQ(1, response.appendData(data, 1));
    ASSERT_EQ(1, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(3, response.appendData(data, 3));
    ASSERT_EQ(4, response.getLength());
    ASSERT_EQ(0x01, buffer[0]);
    ASSERT_EQ(0x01, buffer[1]);
    ASSERT_EQ(0x02, buffer[2]);
    ASSERT_EQ(0x03, buffer[3]);
    ASSERT_EQ(0, response.appendData(data, 1));
    ASSERT_EQ(4, response.getLength());
}

/**
 * \desc
 * Verifies, that the pointer returned by getData() points to the latest byte appended to a
 * response.
 */
TEST_F(PositiveResponseTest, GetData)
{
    uint8_t buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    response.init(buffer, sizeof(buffer));
    ASSERT_EQ(&buffer[0], response.getData());
    response.appendUint8(0x01);
    ASSERT_EQ(&buffer[1], response.getData());
    response.appendUint8(0x01);
    ASSERT_EQ(&buffer[2], response.getData());
    response.appendUint8(0x01);
    ASSERT_EQ(&buffer[3], response.getData());
}

/**
 * \desc
 * Verifies that getAvailableDataLength() returns the number of bytes that can still be
 * appended to a response.
 */
TEST_F(PositiveResponseTest, GetAvailableDataLength)
{
    ASSERT_EQ(0, response.getAvailableDataLength());
    uint8_t buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    response.init(buffer, sizeof(buffer));
    ASSERT_EQ(4, response.getAvailableDataLength());
    response.appendUint8(0x01);
    ASSERT_EQ(3, response.getAvailableDataLength());
    response.appendUint8(0x01);
    ASSERT_EQ(2, response.getAvailableDataLength());
    response.appendUint8(0x01);
    ASSERT_EQ(1, response.getAvailableDataLength());
    response.appendUint8(0x01);
    ASSERT_EQ(0, response.getAvailableDataLength());
}

/**
 * \desc
 * Verifies that increaseDataLength() sets the correct internal size of the response and asserts
 * if it exceeds the number of available bytes.
 */
TEST_F(PositiveResponseTest, IncreaseDataLength)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    uint8_t buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    response.init(buffer, sizeof(buffer));
    ASSERT_EQ(4, response.getAvailableDataLength());
    ASSERT_EQ(0, response.getLength());

    // Increasing past available size will assert.
    ASSERT_THROW(response.increaseDataLength(5), ::estd::assert_exception);

    ASSERT_EQ(1, response.increaseDataLength(1));
    ASSERT_EQ(3, response.getAvailableDataLength());
    ASSERT_EQ(1, response.getLength());

    ASSERT_EQ(4, response.increaseDataLength(3));
    ASSERT_EQ(0, response.getAvailableDataLength());
    ASSERT_EQ(4, response.getLength());

    // Increasing past available size will assert.
    ASSERT_THROW(response.increaseDataLength(1), ::estd::assert_exception);
}

} // namespace
