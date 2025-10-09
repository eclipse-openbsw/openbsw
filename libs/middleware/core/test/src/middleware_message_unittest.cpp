// Copyright 2025 BMW AG

#include "middleware/core/middleware_message.h"
#include "middleware/core/types.h"

#include <etl/limits.h>

#include <gtest/gtest.h>

namespace middleware
{
namespace core
{
namespace test
{

class MiddlewareMessageTest : public ::testing::Test
{
public:
    static bool checkMsgHeader(MiddlewareMessage const& lhs, MiddlewareMessage const& rhs)
    {
        MiddlewareMessage::Header const& lhsHeader = lhs.getHeader();
        MiddlewareMessage::Header const& rhsHeader = rhs.getHeader();
        return lhs.getSourceClusterId() == rhs.getSourceClusterId()
               && lhs.getTargetClusterId() == rhs.getTargetClusterId()
               && lhsHeader.serviceId == rhsHeader.serviceId
               && lhsHeader.memberId == rhsHeader.memberId
               && lhsHeader.serviceInstanceId == rhsHeader.serviceInstanceId
               && lhs.getAddressId() == rhs.getAddressId()
               && lhsHeader.requestId == rhsHeader.requestId && lhs.getFlags() == rhs.getFlags()
               && lhs.isSkeletonTarget() == rhs.isSkeletonTarget()
               && lhs.isProxyTarget() == rhs.isProxyTarget() && lhs.hasError() == rhs.hasError()
               && lhs.isEvent() == rhs.isEvent() && lhs.hasOutArgs() == rhs.hasOutArgs();
    }
};

/**
 * \brief Test the creation of a skeleton response
 *
 */
TEST_F(MiddlewareMessageTest, TestSkeletonResponseCreation)
{
    // ARRANGE
    MiddlewareMessage::Header const header{0x0FD9U, 0x8001U, 0x0010U, 0x0002U};
    auto const sourceClusterId = static_cast<ClusterId>(0xA0U);
    auto const targetClusterId = static_cast<ClusterId>(0x10U);
    AddressId const addressId  = 0x02U;
    MiddlewareMessage msg
        = MiddlewareMessage::createResponse(header, sourceClusterId, targetClusterId, addressId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.hasOutArgs());
    EXPECT_TRUE(msg.isProxyTarget());
    EXPECT_FALSE(msg.isSkeletonTarget());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.hasError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the creation of a proxy request
 *
 */
TEST_F(MiddlewareMessageTest, TestProxyRequestCreation)
{
    // ARRANGE
    MiddlewareMessage::Header const header{0x0237U, 0x0051U, 0x0610U, 0x0011U};
    auto const sourceClusterId = static_cast<ClusterId>(0xA6U);
    auto const targetClusterId = static_cast<ClusterId>(0x23U);
    AddressId const addressId  = 0x05U;
    MiddlewareMessage msg
        = MiddlewareMessage::createRequest(header, sourceClusterId, targetClusterId, addressId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isSkeletonTarget());
    EXPECT_TRUE(msg.hasOutArgs());
    EXPECT_FALSE(msg.isProxyTarget());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.hasError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the creation of a proxy fire and forget request
 *
 */
TEST_F(MiddlewareMessageTest, TestProxyFireAndForgetRequestCreation)
{
    // ARRANGE
    ServiceId const serviceId{0x0089U};
    MemberId const memberId{0x0021U};
    InstanceId const instanceId{0x0710U};
    auto const sourceClusterId = static_cast<ClusterId>(0x06U);
    auto const targetClusterId = static_cast<ClusterId>(0x87U);
    MiddlewareMessage msg      = MiddlewareMessage::createFireAndForgetRequest(
        serviceId, memberId, instanceId, sourceClusterId, targetClusterId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isSkeletonTarget());
    EXPECT_FALSE(msg.hasOutArgs());
    EXPECT_FALSE(msg.isProxyTarget());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_FALSE(msg.hasError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the creation of an error response
 *
 */
TEST_F(MiddlewareMessageTest, TestErrorResponseCreation)
{
    // ARRANGE
    MiddlewareMessage::Header const header{0x1CD7U, 0x005DU, 0x0609U, 0x0000U};
    auto const sourceClusterId = static_cast<ClusterId>(0xC3U);
    auto const targetClusterId = static_cast<ClusterId>(0xF1U);
    AddressId const addressId  = 0x29U;
    ErrorState error           = ErrorState::SerializationError;
    MiddlewareMessage msg      = MiddlewareMessage::createErrorResponse(
        header, sourceClusterId, targetClusterId, addressId, error);

    // ACT && ASSERT
    EXPECT_TRUE(msg.hasError());
    EXPECT_TRUE(msg.isProxyTarget());
    EXPECT_FALSE(msg.hasOutArgs());
    EXPECT_FALSE(msg.isSkeletonTarget());
    EXPECT_FALSE(msg.isEvent());
    EXPECT_EQ(msg.getErrorState(), error);
}

/**
 * \brief Test default constructor, which currently needs to be empty.
 *        What we do here is use new-placement operator and then check
 *        if the values haven't been changed by the default constructor.
 */
TEST_F(MiddlewareMessageTest, TestDefaultConstructor)
{
    // ARRAGE - Create a ProxyRequest
    MiddlewareMessage::Header const header{0xABD7U, 0xA951U, 0xC910U, 0x5011U};
    auto const sourceClusterId = static_cast<ClusterId>(0xEDU);
    auto const targetClusterId = static_cast<ClusterId>(0x81U);
    AddressId const addressId  = 0x36U;
    MiddlewareMessage msg
        = MiddlewareMessage::createRequest(header, sourceClusterId, targetClusterId, addressId);

    MiddlewareMessage const oldMsg = msg;

    // ACT
    // Default constructor will be called and shouldn't change the pre-allocated values
    new (&msg) MiddlewareMessage();

    // ASSERT
    EXPECT_TRUE(checkMsgHeader(oldMsg, msg));
}

/**
 * \brief Test the creation of an event message
 *
 */
TEST_F(MiddlewareMessageTest, TestEventCreation)
{
    // ARRANGE
    ServiceId const serviceId{0x018FU};
    MemberId const memberId{0x0241U};
    InstanceId const instanceId{0x010BU};
    auto const sourceClusterId = static_cast<ClusterId>(0x86U);
    auto const targetClusterId = static_cast<ClusterId>(0x90U);
    MiddlewareMessage msg
        = MiddlewareMessage::createEvent(serviceId, memberId, instanceId, sourceClusterId);
    msg.setTargetClusterId(targetClusterId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isEvent());
    EXPECT_TRUE(msg.isProxyTarget());
    EXPECT_FALSE(msg.isSkeletonTarget());
    EXPECT_FALSE(msg.hasError());
    EXPECT_FALSE(msg.hasOutArgs());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

} // namespace test
} // namespace core
} // namespace middleware
