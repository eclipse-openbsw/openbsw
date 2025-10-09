// Copyright 2025 BMW AG

#include "middleware/core/Message.h"
#include "middleware/core/types.h"

#include <etl/limits.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace middleware
{
namespace core
{
namespace test
{

MATCHER_P(CheckMsgHeader, expected, "Message headers did not match")
{
    Message::Header const& argHeader      = arg.getHeader();
    Message::Header const& expectedHeader = expected.getHeader();
    return argHeader.srcClusterId == expectedHeader.srcClusterId
           && argHeader.tgtClusterId == expectedHeader.tgtClusterId
           && argHeader.serviceId == expectedHeader.serviceId
           && argHeader.memberId == expectedHeader.memberId
           && argHeader.serviceInstanceId == expectedHeader.serviceInstanceId
           && argHeader.addressId == expectedHeader.addressId
           && argHeader.requestId == expectedHeader.requestId;
}

class MiddlewareMessageTest : public ::testing::Test
{};

/**
 * \brief Test the creation of a skeleton response
 *
 */
TEST_F(MiddlewareMessageTest, TestSkeletonResponseCreation)
{
    // ARRANGE
    uint16_t const serviceId      = 0x0FD9U;
    uint16_t const memberId       = 0x8001U;
    uint16_t const requestId      = 0x0010U;
    uint16_t const instanceId     = 0x0002U;
    uint8_t const sourceClusterId = 0xA0U;
    uint8_t const targetClusterId = 0x10U;
    uint8_t const addressId       = 0x02U;
    Message msg                   = Message::createResponse(
        serviceId, memberId, requestId, instanceId, sourceClusterId, targetClusterId, addressId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isResponse());
    EXPECT_FALSE(msg.isRequest());
    EXPECT_FALSE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.isError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the creation of a proxy request
 *
 */
TEST_F(MiddlewareMessageTest, TestProxyRequestCreation)
{
    // ARRANGE
    uint16_t const serviceId      = 0x0237U;
    uint16_t const memberId       = 0x0051U;
    uint16_t const requestId      = 0x0610U;
    uint16_t const instanceId     = 0x0011U;
    uint8_t const sourceClusterId = 0xA6U;
    uint8_t const targetClusterId = 0x23U;
    uint8_t const addressId       = 0x05U;
    Message msg                   = Message::createRequest(
        serviceId, memberId, requestId, instanceId, sourceClusterId, targetClusterId, addressId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isRequest());
    EXPECT_FALSE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.isResponse());
    EXPECT_FALSE(msg.isError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the creation of a proxy fire and forget request
 *
 */
TEST_F(MiddlewareMessageTest, TestProxyFireAndForgetRequestCreation)
{
    // ARRANGE
    uint16_t const serviceId{0x0089U};
    uint16_t const memberId{0x0021U};
    uint16_t const instanceId{0x0710U};
    uint8_t const sourceClusterId = 0x06U;
    uint8_t const targetClusterId = 0x87U;
    Message msg                   = Message::createFireAndForgetRequest(
        serviceId, memberId, instanceId, sourceClusterId, targetClusterId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isRequest());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.isResponse());
    EXPECT_FALSE(msg.isError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the creation of an error response
 *
 */
TEST_F(MiddlewareMessageTest, TestErrorResponseCreation)
{
    // ARRANGE
    uint16_t const serviceId      = 0x1CD7U;
    uint16_t const memberId       = 0x005DU;
    uint16_t const requestId      = 0x0609U;
    uint16_t const instanceId     = 0x0000U;
    uint8_t const sourceClusterId = 0xC3U;
    uint8_t const targetClusterId = 0xF1U;
    uint8_t const addressId       = 0x29U;
    ErrorState error              = ErrorState::SerializationError;
    Message msg                   = Message::createErrorResponse(
        serviceId,
        memberId,
        requestId,
        instanceId,
        sourceClusterId,
        targetClusterId,
        addressId,
        error);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isError());
    EXPECT_FALSE(msg.isRequest());
    EXPECT_FALSE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.isResponse());
    EXPECT_EQ(msg.getErrorState(), error);
}

/**
 * \brief Test the creation of an event message
 *
 */
TEST_F(MiddlewareMessageTest, TestEventCreation)
{
    // ARRANGE
    uint16_t const serviceId{0x018FU};
    uint16_t const memberId{0x0241U};
    uint16_t const instanceId{0x010BU};
    uint8_t const sourceClusterId = 0x86U;
    uint8_t const targetClusterId = 0x90U;
    Message msg = Message::createEvent(serviceId, memberId, instanceId, sourceClusterId);
    msg.setTargetClusterId(targetClusterId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isEvent());
    EXPECT_FALSE(msg.isRequest());
    EXPECT_FALSE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isResponse());
    EXPECT_FALSE(msg.isError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

} // namespace test
} // namespace core
} // namespace middleware
