/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <cstdint>

#include <etl/span.h>

#include "InstancesDatabase.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger/DslLogger.h"
#include "middleware/svc/core/IClusterConnection.h"
#include "middleware/svc/core/Message.h"
#include "middleware/svc/core/SkeletonBase.h"
#include "middleware/svc/core/types.h"

using testing::Exactly;
using testing::NiceMock;

namespace middleware::svc::core::test
{

class Skeleton : public SkeletonBase
{
public:
    HRESULT init(uint16_t instanceId)
    {
        return SkeletonBase::initFromInstancesDatabase(
            instanceId,
            ::etl::span<middleware::svc::core::IInstanceDatabase const* const>(INSTANCESDATABASE));
    }

    HRESULT initEmptyDatabase(uint16_t instanceId)
    {
        return SkeletonBase::initFromInstancesDatabase(
            instanceId,
            ::etl::span<middleware::svc::core::IInstanceDatabase const* const>(
                EMPTYINSTANCESDATABASE));
    }

    HRESULT initBadDatabase(uint16_t instanceId)
    {
        return SkeletonBase::initFromInstancesDatabase(
            instanceId,
            ::etl::span<middleware::svc::core::IInstanceDatabase const* const>(
                BADINSTANCESDATABASE));
    }

    void checkCrossThreadError(uint32_t const initId)
    {
        return SkeletonBase::checkCrossThreadError(initId);
    }

    uint16_t getServiceId() const override { return _serviceId; }

    HRESULT onNewMessageReceived(Message const&) override { return HRESULT::NotImplemented; }

private:
    uint16_t _serviceId{0x10U};
};

class SkeletonBaseTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        _loggerMock.setup();

        const HRESULT res = _skeleton.init(kValidinstanceid);
        EXPECT_EQ(res, HRESULT::Ok);
        EXPECT_TRUE(_skeleton.isInitialized());
    }

    void TearDown() override { _loggerMock.teardown(); }

protected:
    uint16_t const kValidinstanceid{1U};
    uint16_t const kInvalidinstanceid{100U};

    Skeleton _skeleton;
    middleware::logger::test::DslLogger _loggerMock{};
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
    _loggerMock.EXPECT_NO_LOGGING();

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

    _loggerMock.EXPECT_EVENT_LOG(
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

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::SkeletonInitialization,
        HRESULT::InstanceNotFound,
        core::INVALID_CLUSTER_ID,
        skeleton.getServiceId(),
        kValidinstanceid);

    // ACT & ASSERT
    const HRESULT res = skeleton.initEmptyDatabase(kValidinstanceid);

    EXPECT_EQ(res, HRESULT::InstanceNotFound);
    EXPECT_FALSE(skeleton.isInitialized());
}

TEST_F(SkeletonBaseTest, TestInitFromWrongDatabase)
{
    // ARRANGE
    Skeleton skeleton;

    _loggerMock.EXPECT_EVENT_LOG(
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
        _skeleton.getServiceId(),
        0x8001,
        INVALID_REQUEST_ID,
        _skeleton.getInstanceId(),
        _skeleton.getSourceClusterId(),
        tgtClusterId,
        INVALID_ADDRESS_ID);

    _loggerMock.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    const HRESULT res = _skeleton.sendMessage(validMsg);
    EXPECT_EQ(res, HRESULT::Ok);
}

TEST_F(SkeletonBaseTest, TestSendInvalidMessage)
{
    // ARRANGE
    const HRESULT expectedResult = HRESULT::ClusterIdNotFoundOrTransceiverNotRegistered;
    Message invalidMsg           = Message::createResponse(
        _skeleton.getServiceId(),
        0x8001,
        INVALID_REQUEST_ID,
        _skeleton.getInstanceId(),
        _skeleton.getSourceClusterId(),
        _skeleton.getSourceClusterId(),
        INVALID_ADDRESS_ID);

    _loggerMock.EXPECT_EVENT_LOG(
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
    const HRESULT res = _skeleton.sendMessage(invalidMsg);
    EXPECT_EQ(res, expectedResult);
}

TEST_F(SkeletonBaseTest, TestSendMessageFromUnknownSkeleton)
{
    // ARRANGE
    const HRESULT expectedResult = HRESULT::ClusterIdNotFoundOrTransceiverNotRegistered;
    Skeleton skeleton;
    uint8_t const tgtClusterId = static_cast<uint8_t>(2U);
    Message validMsg           = Message::createResponse(
        _skeleton.getServiceId(),
        0x8001,
        INVALID_REQUEST_ID,
        _skeleton.getInstanceId(),
        _skeleton.getSourceClusterId(),
        tgtClusterId,
        INVALID_ADDRESS_ID);

    _loggerMock.EXPECT_EVENT_LOG(
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
    uint8_t const clusterId = _skeleton.getSourceClusterId();

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
    _loggerMock.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    _skeleton.checkCrossThreadError(goodProcess);
}

TEST_F(SkeletonBaseDeathTest, TestCheckCrossThreadErrorWithNotInitSkeleton)
{
    // ARRANGE
    Skeleton skeleton;
    uint32_t const goodProcess = 0U;
    _loggerMock.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    skeleton.checkCrossThreadError(goodProcess);
}

TEST_F(SkeletonBaseDeathTest, TestCheckCrossThreadErrorAssert)
{
    // ARRANGE
    uint32_t const wrongProcess = 1234U;

    // ACT & ASSERT
    EXPECT_ANY_THROW({
        _loggerMock.EXPECT_EVENT_LOG(
            logger::LogLevel::Critical,
            logger::Error::SkeletonCrossThreadViolation,
            _skeleton.getSourceClusterId(),
            _skeleton.getServiceId(),
            _skeleton.getInstanceId(),
            wrongProcess,
            0U);
        _skeleton.checkCrossThreadError(wrongProcess);
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
    EXPECT_FALSE(_skeleton.getClusterConnections().empty());
}

TEST_F(SkeletonBaseTest, TestGetClusterConnectionsFromNotInitSkeleton)
{
    // ARRANGE
    Skeleton skeleton;
    // ACT & ASSERT
    EXPECT_TRUE(skeleton.getClusterConnections().empty());
}

/**
 * @brief A skeleton binds to the database owning its instance, not to one that only reaches it
 */
TEST(SkeletonBaseMultiClusterTest, TestInitUsesConnectionsOfDatabaseOwningInstance)
{
    // ARRANGE
    struct MultiClusterSkeleton : Skeleton
    {
        HRESULT initFrom(
            uint16_t const instanceId,
            ::etl::span<middleware::svc::core::IInstanceDatabase const* const> const& databases)
        {
            return initFromInstancesDatabase(instanceId, databases);
        }
    };

    constexpr uint16_t instanceInCluster0 = 10U;
    constexpr uint16_t instanceInCluster1 = 20U;
    RoutedClusterConnection connection0To0{0U, 0U};
    RoutedClusterConnection connection0To1{0U, 1U};
    RoutedClusterConnection connection1To0{1U, 0U};
    ::etl::array<uint16_t, 1U> const ids0{{instanceInCluster0}};
    ::etl::array<uint16_t, 1U> const ids1{{instanceInCluster1}};
    ::etl::array<IClusterConnection*, 2U> const skeletonConnections0{
        {&connection0To0, &connection0To1}};
    ::etl::array<IClusterConnection*, 1U> const skeletonConnections1{{&connection1To0}};
    ClusterInstancesDatabase const database0{0U, ids0, skeletonConnections0, {}};
    ClusterInstancesDatabase const database1{1U, ids1, skeletonConnections1, {}};
    ::etl::array<IInstanceDatabase const*, 2U> const databases{{&database0, &database1}};
    ::etl::span<IInstanceDatabase const* const> const span(databases);
    MultiClusterSkeleton skeleton0;
    MultiClusterSkeleton skeleton1;

    // ACT & ASSERT
    EXPECT_EQ(skeleton0.initFrom(instanceInCluster0, span), HRESULT::Ok);
    ASSERT_EQ(skeleton0.getClusterConnections().size(), 2U);
    EXPECT_EQ(skeleton0.getClusterConnections()[0], &connection0To0);
    EXPECT_EQ(skeleton0.getClusterConnections()[1], &connection0To1);
    EXPECT_EQ(skeleton1.initFrom(instanceInCluster1, span), HRESULT::Ok);
    ASSERT_EQ(skeleton1.getClusterConnections().size(), 1U);
    EXPECT_EQ(skeleton1.getClusterConnections()[0], &connection1To0);
}

} // namespace middleware::svc::core::test
