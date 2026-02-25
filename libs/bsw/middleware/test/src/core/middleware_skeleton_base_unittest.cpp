#include <stdint.h>

#include <etl/span.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger/DslLogger.h"
#include "middleware/core/IClusterConnection.h"
#include "middleware/core/Message.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"
#include "middleware_instances_database.h"

using testing::_;
using testing::Exactly;
using testing::NiceMock;

namespace middleware
{
namespace core
{
namespace test
{

class Skeleton : public SkeletonBase
{
public:
    HRESULT init(uint16_t instanceId)
    {
        return SkeletonBase::initFromInstancesDatabase(
            instanceId,
            etl::span<middleware::core::IInstanceDatabase const* const>(INSTANCESDATABASE));
    }

    HRESULT initEmptyDatabase(uint16_t instanceId)
    {
        return SkeletonBase::initFromInstancesDatabase(
            instanceId,
            etl::span<middleware::core::IInstanceDatabase const* const>(EMPTYINSTANCESDATABASE));
    }

    HRESULT initBadDatabase(uint16_t instanceId)
    {
        return SkeletonBase::initFromInstancesDatabase(
            instanceId,
            etl::span<middleware::core::IInstanceDatabase const* const>(BADINSTANCESDATABASE));
    }

    void checkCrossThreadError(uint32_t const initId)
    {
        return SkeletonBase::checkCrossThreadError(initId);
    }

    uint16_t getServiceId() const override { return serviceId_; }

    HRESULT onNewMessageReceived(Message const&) override { return HRESULT::NotImplemented; }

private:
    uint16_t serviceId_{0x10U};
};

class SkeletonBaseTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        logger_mock_.setup();

        const HRESULT res = skeleton_.init(kValidinstanceid);
        EXPECT_EQ(res, HRESULT::Ok);
        EXPECT_TRUE(skeleton_.isInitialized());
    }

    void TearDown() override { logger_mock_.teardown(); }

protected:
    uint16_t const kValidinstanceid{1U};
    uint16_t const kInvalidinstanceid{100U};

    Skeleton skeleton_;
    middleware::logger::test::DslLogger logger_mock_{};
};

using SkeletonBaseDeathTest = SkeletonBaseTest;

/**
 * @brief Test initialization from database
 *        Test cases:
 *        - A valid init
 *        - An init with an invalidInstanceId
 *        - [MISSING] An init with an already used instanceId
 *        - Init from an empty Instances Database
 *        - Init from a bad Instances Database
 *         - Reinit skeleton
 */
TEST_F(SkeletonBaseTest, TestInitFromDatabase)
{
    // ARRANGE
    Skeleton skeleton;
    logger_mock_.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    HRESULT res = skeleton.init(kValidinstanceid);
    EXPECT_EQ(res, HRESULT::Ok);
    EXPECT_TRUE(skeleton.isInitialized());

    res = skeleton.init(kValidinstanceid);
    EXPECT_EQ(res, HRESULT::Ok);
    EXPECT_TRUE(skeleton.isInitialized());
}

TEST_F(SkeletonBaseTest, TestInitFromDatabaseWithInvalidInstanceId)
{
    // ARRANGE
    Skeleton skeleton;

    logger_mock_.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::SkeletonInitialization,
        HRESULT::InstanceNotFound,
        core::INVALID_CLUSTER_ID,
        skeleton.getServiceId(),
        kInvalidinstanceid);

    // ACT & ASSERT
    const HRESULT res = skeleton.init(kInvalidinstanceid);

    EXPECT_EQ(res, HRESULT::InstanceNotFound);
    EXPECT_FALSE(skeleton.isInitialized());
}

TEST_F(SkeletonBaseTest, TestInitWithEmptyDatabase)
{
    // ARRANGE
    Skeleton skeleton;

    logger_mock_.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::SkeletonInitialization,
        HRESULT::NoClientsAvailable,
        core::INVALID_CLUSTER_ID,
        skeleton.getServiceId(),
        kValidinstanceid);

    // ACT & ASSERT
    const HRESULT res = skeleton.initEmptyDatabase(kValidinstanceid);

    EXPECT_EQ(res, HRESULT::NoClientsAvailable);
    EXPECT_FALSE(skeleton.isInitialized());
}

TEST_F(SkeletonBaseTest, TestInitFromWrongDatabase)
{
    // ARRANGE
    Skeleton skeleton;

    logger_mock_.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::SkeletonInitialization,
        HRESULT::TransceiverInitializationFailed,
        core::INVALID_CLUSTER_ID,
        skeleton.getServiceId(),
        kValidinstanceid);

    // ACT & ASSERT
    const HRESULT res = skeleton.initBadDatabase(kValidinstanceid);

    EXPECT_EQ(res, HRESULT::TransceiverInitializationFailed);
    EXPECT_FALSE(skeleton.isInitialized());
}

/**
 * @brief Test sendMessage
 *        Test cases:
 *        - Valid  Target Cluster
 *        - Invalid Target Cluster
 *
 */
TEST_F(SkeletonBaseTest, TestSendMessage)
{
    // ARRANGE
    auto const tgtClusterId = static_cast<uint8_t>(2U);
    Message validMsg        = Message::createResponse(
        skeleton_.getServiceId(),
        0x8001,
        INVALID_REQUEST_ID,
        skeleton_.getInstanceId(),
        skeleton_.getSourceClusterId(),
        tgtClusterId,
        INVALID_ADDRESS_ID);

    logger_mock_.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    const HRESULT res = skeleton_.sendMessage(validMsg);
    EXPECT_EQ(res, HRESULT::Ok);
}

TEST_F(SkeletonBaseTest, TestSendInvalidMessage)
{
    // ARRANGE
    const HRESULT expectedResult = HRESULT::ClusterIdNotFoundOrTransceiverNotRegistered;
    Message invalidMsg           = Message::createResponse(
        skeleton_.getServiceId(),
        0x8001,
        INVALID_REQUEST_ID,
        skeleton_.getInstanceId(),
        skeleton_.getSourceClusterId(),
        skeleton_.getSourceClusterId(),
        INVALID_ADDRESS_ID);

    logger_mock_.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::SendMessage,
        expectedResult,
        invalidMsg.getHeader().srcClusterId,
        invalidMsg.getHeader().tgtClusterId,
        invalidMsg.getHeader().serviceId,
        invalidMsg.getHeader().serviceInstanceId,
        invalidMsg.getHeader().memberId,
        invalidMsg.getHeader().requestId);

    // ACT & ASSERT
    const HRESULT res = skeleton_.sendMessage(invalidMsg);
    EXPECT_EQ(res, expectedResult);
}

TEST_F(SkeletonBaseTest, TestSendMessageFromUnknownSkeleton)
{
    // ARRANGE
    const HRESULT expectedResult = HRESULT::ClusterIdNotFoundOrTransceiverNotRegistered;
    Skeleton skeleton;
    uint8_t const tgtClusterId = static_cast<uint8_t>(2U);
    Message validMsg           = Message::createResponse(
        skeleton_.getServiceId(),
        0x8001,
        INVALID_REQUEST_ID,
        skeleton_.getInstanceId(),
        skeleton_.getSourceClusterId(),
        tgtClusterId,
        INVALID_ADDRESS_ID);

    logger_mock_.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::SendMessage,
        expectedResult,
        validMsg.getHeader().srcClusterId,
        validMsg.getHeader().tgtClusterId,
        validMsg.getHeader().serviceId,
        validMsg.getHeader().serviceInstanceId,
        validMsg.getHeader().memberId,
        validMsg.getHeader().requestId);

    // ACT & ASSERT
    const HRESULT res = skeleton.sendMessage(validMsg);
    EXPECT_EQ(res, HRESULT::ClusterIdNotFoundOrTransceiverNotRegistered);
}

/**
 * @brief Test getSourceClusterId
 *        Test cases:
 *        - Inited skeleton
 *        - Not inited skeleton
 *
 */

TEST_F(SkeletonBaseTest, TestGetSourceClusterId)
{
    // ARRANGE

    // ACT
    uint8_t const clusterId = skeleton_.getSourceClusterId();

    // ASSERT
    EXPECT_EQ(clusterId, static_cast<uint8_t>(1U));
}

TEST_F(SkeletonBaseTest, TestGetSourceClusterIdFromNotInitSkeleton)
{
    // ARRANGE
    Skeleton skeleton;

    // ACT
    uint8_t const clusterId = skeleton.getSourceClusterId();

    // ASSERT
    EXPECT_EQ(clusterId, static_cast<uint8_t>(INVALID_CLUSTER_ID));
}

/**
 * @brief Test CheckCrossThreadError
 *        Test cases:
 *        - Process is the same
 *        - Process is NOT the same
 *
 */

TEST_F(SkeletonBaseDeathTest, TestCheckCrossThreadError)
{
    // ARRANGE
    uint32_t const goodProcess = 0U;
    logger_mock_.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    skeleton_.checkCrossThreadError(goodProcess);
}

TEST_F(SkeletonBaseDeathTest, TestCheckCrossThreadErrorWithNotInitSkeleton)
{
    // ARRANGE
    Skeleton skeleton;
    uint32_t const goodProcess = 0U;
    logger_mock_.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    skeleton.checkCrossThreadError(goodProcess);
}

TEST_F(SkeletonBaseDeathTest, TestCheckCrossThreadErrorAssert)
{
    // ARRANGE
    uint32_t const wrongProcess = 1234U;

    // ACT & ASSERT
    EXPECT_ANY_THROW({
        logger_mock_.EXPECT_EVENT_LOG(
            logger::LogLevel::Critical,
            logger::Error::SkeletonCrossThreadViolation,
            skeleton_.getSourceClusterId(),
            skeleton_.getServiceId(),
            skeleton_.getInstanceId(),
            wrongProcess,
            0U);
        skeleton_.checkCrossThreadError(wrongProcess);
    });
}

/**
 * @brief Test getClusterConnections
 *        Test cases:
 *        - Inited skeleton
 *        - Not inited skeleton
 */
TEST_F(SkeletonBaseTest, TestGetClusterConnections)
{
    // ARRANGE
    // ACT & ASSERT
    EXPECT_FALSE(skeleton_.getClusterConnections().empty());
}

TEST_F(SkeletonBaseTest, TestGetClusterConnectionsFromNotInitSkeleton)
{
    // ARRANGE
    Skeleton skeleton;
    // ACT & ASSERT
    EXPECT_TRUE(skeleton.getClusterConnections().empty());
}

} // namespace test
} // namespace core
} // namespace middleware
