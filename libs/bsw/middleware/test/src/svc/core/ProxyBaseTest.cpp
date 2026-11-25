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
#include <stdlib.h>

#include <etl/span.h>

#include "InstancesDatabase.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger/DslLogger.h"
#include "middleware/svc/core/IClusterConnection.h"
#include "middleware/svc/core/LoggerApi.h"
#include "middleware/svc/core/Message.h"
#include "middleware/svc/core/ProxyBase.h"
#include "middleware/svc/core/types.h"

using testing::Exactly;
using testing::NiceMock;

namespace middleware::svc::core::test
{

class Proxy : public ProxyBase
{
public:
    HRESULT init(uint16_t instanceId, uint8_t clusterId)
    {
        return ProxyBase::initFromInstancesDatabase(
            instanceId, clusterId, ::etl::span<IInstanceDatabase const* const>(INSTANCESDATABASE));
    }

    HRESULT initEmptyDatabase(uint16_t instanceId, uint8_t clusterId)
    {
        return ProxyBase::initFromInstancesDatabase(
            instanceId,
            clusterId,
            ::etl::span<IInstanceDatabase const* const>(EMPTYINSTANCESDATABASE));
    }

    uint8_t getProxySourceClusterId() { return ProxyBase::getSourceClusterId(); }

    void checkCrossThreadError(uint32_t const initId)
    {
        return ProxyBase::checkCrossThreadError(initId);
    }

    uint16_t getServiceId() const override { return _serviceId; }

    HRESULT onNewMessageReceived(Message const&) override { return HRESULT::NotImplemented; }

private:
    uint16_t _serviceId{0x10U};
};

class ProxyBaseTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        _loggerMock.setup();
        const HRESULT res = _proxy.init(kValidinstanceid, kValidclustid);
        EXPECT_EQ(res, HRESULT::Ok);
        EXPECT_TRUE(_proxy.isInitialized());
    }

    void TearDown() override { _loggerMock.teardown(); }

protected:
    uint16_t const kValidinstanceid{1U};
    uint8_t const kValidclustid{static_cast<uint8_t>(1U)};
    uint16_t const kInvalidinstanceid{100U};
    uint8_t const kInvalidclustid{static_cast<uint8_t>(100U)};

    Proxy _proxy;
    middleware::logger::test::DslLogger _loggerMock{};
};

using ProxyBaseDeathTest = ProxyBaseTest;

TEST_F(ProxyBaseTest, TestInitFromDatabase)
{
    // ARRANGE
    Proxy proxy;
    _loggerMock.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    HRESULT res = proxy.init(kValidinstanceid, kValidclustid);
    EXPECT_EQ(res, HRESULT::Ok);

    EXPECT_TRUE(proxy.isInitialized());

    // re-init
    res = proxy.init(kValidinstanceid, kValidclustid);
    EXPECT_EQ(res, HRESULT::Ok);
    EXPECT_TRUE(proxy.isInitialized());
}

TEST_F(ProxyBaseTest, TestInitFromDatabaseWithInvalidInstanceId)
{
    // ARRANGE
    Proxy proxy;

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::ProxyInitialization,
        HRESULT::InstanceNotFound,
        kValidclustid,
        proxy.getServiceId(),
        kInvalidinstanceid);

    // ACT & ASSERT
    const HRESULT res = proxy.init(kInvalidinstanceid, kValidclustid);

    EXPECT_EQ(res, HRESULT::InstanceNotFound);
    EXPECT_FALSE(proxy.isInitialized());
}

TEST_F(ProxyBaseTest, TestInitWithEmptyDatabase)
{
    // ARRANGE
    Proxy proxy;

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::ProxyInitialization,
        HRESULT::InstanceNotFound,
        kValidclustid,
        proxy.getServiceId(),
        kValidinstanceid);

    // ACT & ASSERT
    const HRESULT res = proxy.initEmptyDatabase(kValidinstanceid, kValidclustid);

    EXPECT_EQ(res, HRESULT::InstanceNotFound);
    EXPECT_FALSE(proxy.isInitialized());
}

TEST_F(ProxyBaseTest, TestInitFromDatabaseWithInvalidClusterId)
{
    // ARRANGE
    Proxy proxy;

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::ProxyInitialization,
        HRESULT::TransceiverInitializationFailed,
        kInvalidclustid,
        proxy.getServiceId(),
        kValidinstanceid);

    // ACT & ASSERT
    const HRESULT res = proxy.init(kValidinstanceid, kInvalidclustid);

    EXPECT_EQ(res, HRESULT::TransceiverInitializationFailed);
    EXPECT_FALSE(proxy.isInitialized());
}

TEST_F(ProxyBaseTest, TestGenerateMessageHeaderWithRequestId)
{
    // ARRANGE
    uint16_t const memberId{0x15U};
    uint16_t const requestId{0x05U};

    // ACT
    Message const msg = _proxy.generateMessageHeader(memberId, requestId);

    // ASSERT
    EXPECT_EQ(msg.getHeader().srcClusterId, kValidclustid);
    EXPECT_EQ(msg.getHeader().tgtClusterId, static_cast<uint8_t>(2U)); // Hardcoded for now
    EXPECT_EQ(msg.getHeader().serviceId, _proxy.getServiceId());
    EXPECT_EQ(msg.getHeader().memberId, memberId);
    EXPECT_EQ(msg.getHeader().serviceInstanceId, _proxy.getInstanceId());
    EXPECT_EQ(msg.getHeader().addressId, _proxy.getAddressId());
    EXPECT_EQ(msg.getHeader().requestId, requestId);
    EXPECT_TRUE(msg.isRequest());
    EXPECT_FALSE(msg.isEvent());
}

TEST_F(ProxyBaseTest, TestGenerateMessageHeaderWithInvalidRequestId)
{
    // ARRANGE
    uint16_t const memberId{0x15U};

    // ACT
    Message msg = _proxy.generateMessageHeader(memberId, INVALID_REQUEST_ID);

    // ASSERT
    EXPECT_EQ(msg.getHeader().srcClusterId, kValidclustid);
    EXPECT_EQ(msg.getHeader().tgtClusterId, static_cast<uint8_t>(2U)); // Hardcoded for now
    EXPECT_EQ(msg.getHeader().serviceId, _proxy.getServiceId());
    EXPECT_EQ(msg.getHeader().memberId, memberId);
    EXPECT_EQ(msg.getHeader().serviceInstanceId, _proxy.getInstanceId());
    EXPECT_EQ(msg.getHeader().addressId, _proxy.getAddressId());
    EXPECT_EQ(msg.getHeader().requestId, INVALID_REQUEST_ID);
    EXPECT_TRUE(msg.isFireAndForgetRequest());
    EXPECT_FALSE(msg.isEvent());
}

/**
 * @brief Test generation and message sending
 *        Test cases:
 *        - Successful message sent
 *        - Not initialized
 *        - [MISSING] Too big of a payload
 *        - [MISSING] Queue Full
 *
 */

TEST_F(ProxyBaseTest, TestGenerateAndSendMessage)
{
    // ARRANGE
    uint16_t const memberId{0x15U};
    uint16_t const requestId{0x05U};

    // ACT
    Message msg = _proxy.generateMessageHeader(memberId, requestId);

    // ASSERT
    EXPECT_EQ(_proxy.sendMessage(msg), HRESULT::Ok);
}

TEST_F(ProxyBaseTest, TestSendMessageWithNotInitProxy)
{
    // ARRANGE
    Proxy proxy;
    Message msg
        = Message::createRequest(proxy.getServiceId(), 0x01U, 0x01U, 0x01U, 0x01U, 0x02U, 0x01U);

    // ASSERT
    EXPECT_EQ(proxy.sendMessage(msg), HRESULT::NotRegistered);
}

TEST_F(ProxyBaseDeathTest, TestCheckCrossThreadErrorWithSameTaskId)
{
    // ARRANGE
    uint32_t const goodProcess = 0U;
    _loggerMock.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    _proxy.checkCrossThreadError(goodProcess);
}

TEST_F(ProxyBaseDeathTest, TestCheckCrossThreadErrorWithNotInitProxy)
{
    // ARRANGE
    Proxy proxy;
    uint32_t const goodProcess = 0U;
    _loggerMock.EXPECT_NO_LOGGING();

    // ACT & ASSERT
    proxy.checkCrossThreadError(goodProcess);
}

TEST_F(ProxyBaseDeathTest, TestCheckCrossThreadErrorAbort)
{
    // ARRANGE
    uint32_t const wrongProcess = 1234U;

    // ACT & ASSERT
    EXPECT_ANY_THROW({
        _loggerMock.EXPECT_EVENT_LOG(
            logger::LogLevel::Critical,
            logger::Error::ProxyCrossThreadViolation,
            _proxy.getProxySourceClusterId(),
            _proxy.getServiceId(),
            _proxy.getInstanceId(),
            wrongProcess,
            0U);
        _proxy.checkCrossThreadError(wrongProcess);
    });
}

/**
 * @brief Services deployed in several clusters
 *        Test cases:
 *        - The database of the requested source cluster is selected
 *        - The connection towards the cluster hosting the instance is selected
 */
TEST(ProxyBaseMultiClusterTest, TestInitSelectsConnectionOfSourceClusterTowardsInstance)
{
    // ARRANGE
    struct MultiClusterProxy : Proxy
    {
        HRESULT initFrom(
            uint16_t const instanceId,
            uint8_t const sourceCluster,
            ::etl::span<IInstanceDatabase const* const> const& databases)
        {
            return initFromInstancesDatabase(instanceId, sourceCluster, databases);
        }

        IClusterConnection* connection() const { return _connection; }
    };

    constexpr uint16_t instanceInCluster0 = 10U;
    constexpr uint16_t instanceInCluster1 = 20U;
    RoutedClusterConnection connection0To0{0U, 0U};
    RoutedClusterConnection connection0To1{0U, 1U};
    RoutedClusterConnection connection1To0{1U, 0U};
    RoutedClusterConnection connection1To1{1U, 1U};
    ::etl::array<ProxyRoute, 2U> const routes0{
        {{instanceInCluster0, &connection0To0}, {instanceInCluster1, &connection0To1}}};
    ::etl::array<ProxyRoute, 2U> const routes1{
        {{instanceInCluster0, &connection1To0}, {instanceInCluster1, &connection1To1}}};
    ClusterInstancesDatabase const database0{0U, {}, {}, routes0};
    ClusterInstancesDatabase const database1{1U, {}, {}, routes1};
    ::etl::array<IInstanceDatabase const*, 2U> const databases{{&database0, &database1}};
    ::etl::span<IInstanceDatabase const* const> const span(databases);
    MultiClusterProxy proxy;

    // ACT & ASSERT
    EXPECT_EQ(proxy.initFrom(instanceInCluster0, 0U, span), HRESULT::Ok);
    EXPECT_EQ(proxy.connection(), &connection0To0);
    EXPECT_EQ(proxy.initFrom(instanceInCluster1, 0U, span), HRESULT::Ok);
    EXPECT_EQ(proxy.connection(), &connection0To1);
    EXPECT_EQ(proxy.initFrom(instanceInCluster0, 1U, span), HRESULT::Ok);
    EXPECT_EQ(proxy.connection(), &connection1To0);
    EXPECT_EQ(proxy.initFrom(instanceInCluster1, 1U, span), HRESULT::Ok);
    EXPECT_EQ(proxy.connection(), &connection1To1);
}

/**
 * @brief Instance deployed in a cluster that the source cluster has no route to
 *        Test cases:
 *        - HRESULT::InstanceNotFound is returned and the proxy stays uninitialized
 */
TEST_F(ProxyBaseTest, TestInitWithInstanceUnreachableFromSourceCluster)
{
    // ARRANGE
    struct MultiClusterProxy : Proxy
    {
        HRESULT initFrom(
            uint16_t const instanceId,
            uint8_t const sourceCluster,
            ::etl::span<IInstanceDatabase const* const> const& databases)
        {
            return initFromInstancesDatabase(instanceId, sourceCluster, databases);
        }
    };

    constexpr uint16_t unreachableInstance = 20U;
    RoutedClusterConnection connection1To1{1U, 1U};
    ::etl::array<ProxyRoute, 1U> const routes1{{{unreachableInstance, &connection1To1}}};
    ClusterInstancesDatabase const database0{0U, {}, {}, {}};
    ClusterInstancesDatabase const database1{1U, {}, {}, routes1};
    ::etl::array<IInstanceDatabase const*, 2U> const databases{{&database0, &database1}};
    ::etl::span<IInstanceDatabase const* const> const span(databases);
    MultiClusterProxy proxy;

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Critical,
        logger::Error::ProxyInitialization,
        HRESULT::InstanceNotFound,
        static_cast<uint8_t>(0U),
        proxy.getServiceId(),
        unreachableInstance);

    // ACT & ASSERT
    EXPECT_EQ(proxy.initFrom(unreachableInstance, 0U, span), HRESULT::InstanceNotFound);
    EXPECT_FALSE(proxy.isInitialized());
}

} // namespace middleware::svc::core::test
