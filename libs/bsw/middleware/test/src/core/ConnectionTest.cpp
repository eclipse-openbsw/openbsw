/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <cstdint>

#include <etl/optional.h>

#include "InstancesDatabase.h"
#include "Proxy.h"
#include "Skeleton.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger/DslLogger.h"
#include "middleware/core/ClusterConnection.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"

using testing::Exactly;
using testing::NiceMock;

namespace middleware::core::test
{

struct ClusterConfigurationMockBase
{
    static uint8_t const sourceClusterId{1};
    static uint8_t const targetClusterId{2};

    void setNextHRESULT(::middleware::core::HRESULT const& ret, std::uint8_t times = 1)
    {
        _mockReturnCounter = times;
        returnHRESULT      = ret;
    }

    void setNextWriteResult(bool ret) { returnWrite = ret; }

    ::etl::optional<::middleware::core::Message> getLastReceivedMessage()
    {
        return messageReceived;
    }

    HRESULT dispatchMessage(::middleware::core::Message const& msg) const
    {
        messageReceived = msg;
        return getResultInternal();
    }

    HRESULT subscribe(ProxyBase&, uint16_t const) { return getResultInternal(); }

    HRESULT subscribe(SkeletonBase&, uint16_t const) { return getResultInternal(); }

    uint8_t getSourceClusterId() const { return sourceClusterId; }

    uint8_t getTargetClusterId() const { return targetClusterId; }

    bool write(Message const&) const { return returnWrite; }

private:
    inline ::middleware::core::HRESULT getResultInternal() const
    {
        return (_mockReturnCounter-- >= 1) ? returnHRESULT : ::middleware::core::HRESULT::Ok;
    }

    ::middleware::core::HRESULT returnHRESULT{::middleware::core::HRESULT::Ok};
    mutable std::uint8_t _mockReturnCounter{1};
    bool returnWrite{true};
    mutable ::etl::optional<::middleware::core::Message> messageReceived;
};

struct TimeoutTransceiverCounter
{
    void up() { _cnt++; }

    void down() { (_cnt > 0) ? _cnt-- : _cnt = 0; }

    std::size_t getValue() const { return _cnt; }

    bool hasBeenTriggered() { return _triggered; }

    void updateTimeouts() { _triggered = true; }

private:
    std::size_t _cnt{0};
    bool _triggered{false};
};

struct ClusterConnectionConfigurationMock
: public IClusterConnectionConfiguration
, ClusterConfigurationMockBase
, TimeoutTransceiverCounter
{
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) override
    {
        return ClusterConfigurationMockBase::subscribe(proxy, serviceInstanceId);
    }

    void unsubscribe(ProxyBase&, uint16_t const) override {}

    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) override
    {
        return ClusterConfigurationMockBase::subscribe(skeleton, serviceInstanceId);
    }

    void unsubscribe(SkeletonBase&, uint16_t const) override {}

    HRESULT dispatchMessage(Message const& msg) const override
    {
        return ClusterConfigurationMockBase::dispatchMessage(msg);
    }

    uint8_t getSourceClusterId() const override
    {
        return ClusterConfigurationMockBase::getSourceClusterId();
    }

    uint8_t getTargetClusterId() const override
    {
        return ClusterConfigurationMockBase::getTargetClusterId();
    }

    bool write(Message const& msg) const override
    {
        return ClusterConfigurationMockBase::write(msg);
    }

    std::size_t registeredTransceiversCount(uint16_t const) const override
    {
        return TimeoutTransceiverCounter::getValue();
    }

    void registerTimeoutTransceiver(ITimeoutHandler&) override { TimeoutTransceiverCounter::up(); }

    void unregisterTimeoutTransceiver(ITimeoutHandler&) override
    {
        TimeoutTransceiverCounter::down();
    }

    void updateTimeouts() override { TimeoutTransceiverCounter::updateTimeouts(); }
};

class ConnectionBaseTest : public ::testing::Test
{
public:
    void SetUp() override { _loggerMock.setup(); }

    void TearDown() override { _loggerMock.teardown(); }

    middleware::logger::test::DslLogger _loggerMock{};
};

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeProxyOnly)
{
    Proxy proxyInstance(1, 2, 4);
    ClusterConnectionConfigurationMock configuration;

    ClusterConnection connection(configuration);
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connection.subscribe(proxyInstance, 1));
    EXPECT_NO_THROW(connection.unsubscribe(proxyInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeSkeletonOnly)
{
    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock configuration;

    ClusterConnection connection(configuration);
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connection.subscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(connection.unsubscribe(skeletonInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeBidirectional)
{
    Proxy proxyInstance(1, 2, 4);
    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock configuration;

    ClusterConnection connection(configuration);
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connection.subscribe(skeletonInstance, 1));
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connection.subscribe(proxyInstance, 1));
    EXPECT_NO_THROW(connection.unsubscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(connection.unsubscribe(proxyInstance, 1));
}

TEST_F(ConnectionBaseTest, SendMessageSameClusterNoError)
{
    Message msg = Message::createRequest(
        1,
        123,
        321,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::sourceClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));
    EXPECT_EQ(::middleware::core::HRESULT::Ok, ptrToBase->sendMessage(msg));
}

TEST_F(ConnectionBaseTest, SendMessageClusterToClusterNoError)
{
    Message msg = Message::createRequest(
        1,
        123,
        321,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::targetClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));
    EXPECT_EQ(::middleware::core::HRESULT::Ok, ptrToBase->sendMessage(msg));
}

TEST_F(ConnectionBaseTest, SendMessageClusterToClusterFailed)
{
    Message msg = Message::createRequest(
        1,
        123,
        321,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::targetClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // next invocation of write() on the cluster connection config will return false
    confSkeletonOnly.setNextWriteResult(false);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::SendMessage,
        HRESULT::QueueFull,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    // expecting fall-through, returning the HRESULT from the initialization
    EXPECT_EQ(::middleware::core::HRESULT::QueueFull, ptrToBase->sendMessage(msg));
}

TEST_F(ConnectionBaseTest, SendMessageSameClusterServiceNotFound)
{
    Message msg = Message::createRequest(
        1,
        123,
        321,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::sourceClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // sendMessage: Setting error code ServiceNotFound exactly one time for the receiving side, but
    // fall back to OK when sending back the error
    confSkeletonOnly.setNextHRESULT(::middleware::core::HRESULT::ServiceNotFound, 1);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::DispatchMessage,
        HRESULT::ServiceNotFound,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    EXPECT_EQ(::middleware::core::HRESULT::ServiceNotFound, ptrToBase->sendMessage(msg));

    auto lastReceivedMsg = confSkeletonOnly.getLastReceivedMessage();
    EXPECT_EQ(lastReceivedMsg.value().getErrorState(), ErrorState::ServiceNotFound);
}

TEST_F(ConnectionBaseTest, SendMessageSameClusterServiceBusy)
{
    Message msg = Message::createRequest(
        1,
        123,
        321,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::sourceClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // sendMessage: Setting error code ServiceNotFound exactly one time for the receiving side, but
    // fall back to OK when sending back the error
    confSkeletonOnly.setNextHRESULT(::middleware::core::HRESULT::ServiceBusy, 1);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::DispatchMessage,
        HRESULT::ServiceBusy,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    EXPECT_EQ(::middleware::core::HRESULT::ServiceBusy, ptrToBase->sendMessage(msg));

    auto lastReceivedMsg = confSkeletonOnly.getLastReceivedMessage();
    EXPECT_EQ(lastReceivedMsg.value().getErrorState(), ErrorState::ServiceBusy);
}

TEST_F(ConnectionBaseTest, SendMessageSameClusterServiceBusyFireAndForget)
{
    Message msg = Message::createRequest(
        1,
        123,
        INVALID_REQUEST_ID,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::sourceClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // sendMessage: Setting error code ServiceNotFound exactly one time for the receiving side, but
    // fall back to OK when sending back the error
    confSkeletonOnly.setNextHRESULT(::middleware::core::HRESULT::ServiceBusy, 1);
    EXPECT_EQ(::middleware::core::HRESULT::ServiceBusy, ptrToBase->sendMessage(msg));
}

TEST_F(ConnectionBaseTest, processMessageFromReceivingSide)
{
    // generated code pops a single message from the queue
    Message msg = Message::createRequest(
        1,
        123,
        321,
        2,
        ClusterConfigurationMockBase::sourceClusterId,
        ClusterConfigurationMockBase::sourceClusterId,
        4);

    Skeleton skeletonInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(ptrToBase->processMessage(msg));

    auto lastReceivedMsg = confSkeletonOnly.getLastReceivedMessage();
    EXPECT_EQ(lastReceivedMsg.value().getErrorState(), ErrorState::NoError);
}

TEST_F(ConnectionBaseTest, ProxyRegistersAsTimeoutTransceiver)
{
    struct ProxyWithTimeout
    : public Proxy
    , public ::middleware::core::ITimeoutHandler
    {
        using Proxy::Proxy;

        void updateTimeouts() override {}
    };

    ProxyWithTimeout proxyInstance(1, 2);
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    EXPECT_NO_THROW(actualConnection.registerTimeoutTransceiver(proxyInstance));
    EXPECT_EQ(1, actualConnection.registeredTransceiversCount(1));
    EXPECT_NO_THROW(actualConnection.updateTimeouts());

    EXPECT_TRUE(confSkeletonOnly.hasBeenTriggered());

    EXPECT_NO_THROW(actualConnection.unregisterTimeoutTransceiver(proxyInstance));
    EXPECT_EQ(0, actualConnection.registeredTransceiversCount(1));
}

TEST_F(ConnectionBaseTest, SyntheticClusterIdGetter)
{
    ClusterConnectionConfigurationMock confSkeletonOnly;
    ::middleware::core::ClusterConnection actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    // expectation: Connection getters are just relaying to the configuration getters
    EXPECT_EQ(ptrToBase->getSourceClusterId(), confSkeletonOnly.getSourceClusterId());
    EXPECT_EQ(ptrToBase->getTargetClusterId(), confSkeletonOnly.getTargetClusterId());
}

} // namespace middleware::core::test
