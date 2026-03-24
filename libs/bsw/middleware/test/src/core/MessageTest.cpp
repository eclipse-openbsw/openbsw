// Copyright 2025 BMW AG

#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"

#include "memory/mock/AllocatorMock.h"

#include <etl/limits.h>

#include <cstring>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace middleware
{
namespace core
{
namespace test
{

struct SmallUserType
{
    uint32_t a{};
    uint32_t b{};
    uint32_t c{};
    uint32_t d{};
};

struct BigUserType
{
    uint64_t a{};
    uint64_t b{};
    uint64_t c{};
    uint64_t d{};
    uint64_t e{};
    uint64_t f{};
    uint64_t g{};
    uint64_t h{};
    uint64_t i{};
};

} // namespace test
} // namespace core
} // namespace middleware

namespace etl
{
template<>
struct is_trivially_copyable<middleware::core::test::SmallUserType> : public etl::true_type
{};

template<>
struct is_trivially_copyable<middleware::core::test::BigUserType> : public etl::true_type
{};
} // namespace etl

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
{
public:
    void SetUp() final { memory::test::AllocatorMock::setAllocatorMock(_allocatorMock); }

    static bool checkMsgHeader(Message const& lhs, Message const& rhs)
    {
        Message::Header const& lhsHeader = lhs.getHeader();
        Message::Header const& rhsHeader = rhs.getHeader();
        return memcmp(&lhsHeader, &rhsHeader, sizeof(Message::Header)) == 0U;
    }

private:
    testing::NiceMock<memory::test::AllocatorMock> _allocatorMock;
};

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
    Message msg = Message::createEvent(serviceId, memberId, instanceId, sourceClusterId);

    // ACT && ASSERT
    EXPECT_TRUE(msg.isEvent());
    EXPECT_FALSE(msg.isRequest());
    EXPECT_FALSE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isResponse());
    EXPECT_FALSE(msg.isError());
    EXPECT_EQ(msg.getErrorState(), ErrorState::NoError);
}

/**
 * \brief Test the default constructor does not modify a pre-initialized message
 *
 */
TEST_F(MiddlewareMessageTest, TestDefaultConstructor)
{
    // ARRANGE - Create a ProxyRequest
    uint8_t const sourceClusterId = 0xEDU;
    uint8_t const targetClusterId = 0x81U;
    uint8_t const addressId       = 0x36U;
    Message msg                   = Message::createRequest(
        0xABD7U, 0xA951U, 0xC910U, 0x5011U, sourceClusterId, targetClusterId, addressId);

    Message const oldMsg = msg;

    // ACT
    // Default constructor will be called and shouldn't change the pre-allocated values
    new (&msg) Message();

    // ASSERT
    EXPECT_TRUE(checkMsgHeader(oldMsg, msg));
}

TEST_F(MiddlewareMessageTest, TestSetInternalPayload)
{
    // ARRANGE
    Message msg = Message::createRequest(0xDDD7U, 0x0891U, 0xCA10U, 0x002DU, 0x0DU, 0x89U, 0x22U);
    SmallUserType const obj{0xFF21U, 0xA351U, 0xABC1U, 0x9181U};

    // ACT
    HRESULT ret          = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<SmallUserType>(msg);

    // ASSERT
    EXPECT_EQ(ret, HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.c, storedObj.c);
    EXPECT_EQ(obj.d, storedObj.d);
}

TEST_F(MiddlewareMessageTest, TestReadRawInternalPayload)
{
    // ARRANGE
    Message msg = Message::createRequest(0xDDD7U, 0x0891U, 0xCA10U, 0x002DU, 0x0DU, 0x89U, 0x22U);
    SmallUserType const obj{0xFF21U, 0xA351U, 0xABC1U, 0x9181U};

    // ACT
    HRESULT ret            = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const returnedObj = MessagePayloadBuilder::readRawPayload(msg);
    SmallUserType storedObj{};
    std::memcpy(&storedObj, returnedObj.data(), sizeof(SmallUserType));

    // ASSERT
    EXPECT_EQ(ret, HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.c, storedObj.c);
    EXPECT_EQ(obj.d, storedObj.d);
}

TEST_F(MiddlewareMessageTest, TestReadRawExternalPayload)
{
    // ARRANGE
    Message msg = Message::createRequest(0xDDD7U, 0x0891U, 0xCA10U, 0x002DU, 0x0DU, 0x89U, 0x22U);
    BigUserType const obj{
        0xF1359EA0221A3749U,
        0x51314BA1F17BCD21U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0xA1B2C3D4E5F60718U,
        0x0918273645546372U};

    // ACT
    HRESULT ret            = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const returnedObj = MessagePayloadBuilder::readRawPayload(msg);
    BigUserType storedObj{};
    std::memcpy(&storedObj, returnedObj.data(), sizeof(BigUserType));

    // ASSERT
    EXPECT_EQ(ret, HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.c, storedObj.c);
    EXPECT_EQ(obj.d, storedObj.d);
    EXPECT_EQ(obj.e, storedObj.e);
    EXPECT_EQ(obj.f, storedObj.f);
    EXPECT_EQ(obj.g, storedObj.g);
    EXPECT_EQ(obj.h, storedObj.h);
    EXPECT_EQ(obj.i, storedObj.i);
}

TEST_F(MiddlewareMessageTest, TestSetExternalPayload)
{
    // ARRANGE
    Message msg = Message::createRequest(0x54D7U, 0x8851U, 0xC990U, 0x5051U, 0x7CU, 0x1DU, 0x66U);
    BigUserType const obj{
        0xF1359EA0221A3749U,
        0x51314BA1F17BCD21U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0xA1B2C3D4E5F60718U,
        0x0918273645546372U};

    // ACT
    HRESULT ret          = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<BigUserType>(msg);

    // ASSERT
    EXPECT_EQ(ret, HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.c, storedObj.c);
    EXPECT_EQ(obj.d, storedObj.d);
    EXPECT_EQ(obj.e, storedObj.e);
    EXPECT_EQ(obj.f, storedObj.f);
    EXPECT_EQ(obj.g, storedObj.g);
    EXPECT_EQ(obj.h, storedObj.h);
    EXPECT_EQ(obj.i, storedObj.i);
}

} // namespace test
} // namespace core
} // namespace middleware
