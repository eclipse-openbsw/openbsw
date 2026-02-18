#include <cstdint>

#include <etl/optional.h>

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
#include "middleware_instances_database.h"
#include "proxy.h"
#include "skeleton.h"

using testing::_;
using testing::Exactly;
using testing::NiceMock;

namespace middleware
{
namespace core
{
namespace test
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

    etl::optional<::middleware::core::Message> getLastReceivedMessage() { return messageReceived; }

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
    mutable etl::optional<::middleware::core::Message> messageReceived;
};

struct ProxyMockWithTimeout
: public ProxyMock
, public ::middleware::core::ITimeout
{
    using ProxyMock::ProxyMock;

    void updateTimeouts() override {}
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

struct ClusterConnectionConfigurationProxyOnlyMock
: public IClusterConnectionConfigurationProxyOnly
, ClusterConfigurationMockBase
{
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) override
    {
        return ClusterConfigurationMockBase::subscribe(proxy, serviceInstanceId);
    }

    void unsubscribe(ProxyBase&, uint16_t const) override {}

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

    std::size_t registeredTransceiversCount(uint16_t const) const override { return 0; }
};

struct ClusterConnectionConfigurationSkeletonOnlyMock
: public IClusterConnectionConfigurationSkeletonOnly
, ClusterConfigurationMockBase
{
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

    std::size_t registeredTransceiversCount(uint16_t const) const override { return 0; }
};

struct ClusterConnectionConfigurationBidirectionalMock
: public IClusterConnectionConfigurationBidirectional
, ClusterConfigurationMockBase
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

    std::size_t registeredTransceiversCount(uint16_t const) const override { return 0; }
};

struct ClusterConnectionConfigurationProxyOnlyTimeoutMock
: public IClusterConnectionConfigurationProxyOnlyWithTimeout
, ClusterConfigurationMockBase
, TimeoutTransceiverCounter
{
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) override
    {
        return ClusterConfigurationMockBase::subscribe(proxy, serviceInstanceId);
    }

    void unsubscribe(ProxyBase&, uint16_t const) override {}

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

    std::size_t registeredTransceiversCount(uint16_t const) const override { return 0; }

    void registerTimeoutTransceiver(ITimeout&) override { TimeoutTransceiverCounter::up(); }

    void unregisterTimeoutTransceiver(ITimeout&) override { TimeoutTransceiverCounter::down(); }

    void updateTimeouts() override { TimeoutTransceiverCounter::updateTimeouts(); }
};

struct ClusterConnectionConfigurationSkeletonOnlyTimeoutMock
: public IClusterConnectionConfigurationSkeletonOnlyWithTimeout
, ClusterConfigurationMockBase
, TimeoutTransceiverCounter
{
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

    void registerTimeoutTransceiver(ITimeout&) override { TimeoutTransceiverCounter::up(); }

    void unregisterTimeoutTransceiver(ITimeout&) override { TimeoutTransceiverCounter::down(); }

    void updateTimeouts() override { TimeoutTransceiverCounter::updateTimeouts(); }
};

struct ClusterConnectionConfigurationBidirectionalTimeoutMock
: public IClusterConnectionConfigurationBidirectionalWithTimeout
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

    void registerTimeoutTransceiver(ITimeout&) override { TimeoutTransceiverCounter::up(); }

    void unregisterTimeoutTransceiver(ITimeout&) override { TimeoutTransceiverCounter::down(); }

    void updateTimeouts() override { TimeoutTransceiverCounter::updateTimeouts(); }
};

class ConnectionBaseTest : public ::testing::Test
{
public:
    void SetUp() override { logger_mock_.setup(); }

    void TearDown() override { logger_mock_.teardown(); }

    middleware::logger::test::DslLogger logger_mock_{};
};

/* Testing final method implementations, implemented in the base classes,
 * not meant to be supported on the public interfaces.
 *
 * For instance: subscribing with a skeleton on a Proxy-Only class
 */
TEST_F(ConnectionBaseTest, ConnectionsNotImplemented)
{
    ProxyMock proxyInstance(1, 2, 4);
    SkeletonMock skeletonInstance(1, 2);

    ClusterConnectionConfigurationProxyOnlyMock confProxyOnly;
    ClusterConnectionNoTimeoutProxyOnly connectionProxyOnly(confProxyOnly);
    EXPECT_EQ(
        ::middleware::core::HRESULT::NotImplemented,
        connectionProxyOnly.subscribe(skeletonInstance, 123));
    EXPECT_NO_THROW(connectionProxyOnly.unsubscribe(skeletonInstance, 123));

    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ClusterConnectionNoTimeoutSkeletonOnly connectionSkeletonOnly(confSkeletonOnly);
    EXPECT_EQ(
        ::middleware::core::HRESULT::NotImplemented,
        connectionSkeletonOnly.subscribe(proxyInstance, 123));
    EXPECT_NO_THROW(connectionSkeletonOnly.unsubscribe(proxyInstance, 123));

    ClusterConnectionConfigurationProxyOnlyTimeoutMock confProxyOnlyTimeout;
    ClusterConnectionProxyOnlyWithTimeout connectionProxyOnlyTimeout(confProxyOnlyTimeout);
    EXPECT_EQ(
        ::middleware::core::HRESULT::NotImplemented,
        connectionProxyOnlyTimeout.subscribe(skeletonInstance, 123));
    EXPECT_NO_THROW(connectionProxyOnlyTimeout.unsubscribe(skeletonInstance, 123));

    ClusterConnectionConfigurationSkeletonOnlyTimeoutMock confSkeletonOnlyTimeout;
    ClusterConnectionSkeletonOnlyWithTimeout connectionSkeletonOnlyTimeout(confSkeletonOnlyTimeout);
    EXPECT_EQ(
        ::middleware::core::HRESULT::NotImplemented,
        connectionSkeletonOnlyTimeout.subscribe(proxyInstance, 123));
    EXPECT_NO_THROW(connectionSkeletonOnlyTimeout.unsubscribe(proxyInstance, 123));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeProxyOnly)
{
    ProxyMock proxyInstance(1, 2, 4);
    ClusterConnectionConfigurationProxyOnlyMock confProxyOnly;

    ClusterConnectionNoTimeoutProxyOnly connectionProxyOnly(confProxyOnly);
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connectionProxyOnly.subscribe(proxyInstance, 1));
    EXPECT_NO_THROW(connectionProxyOnly.unsubscribe(proxyInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeSkeletonOnly)
{
    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;

    ClusterConnectionNoTimeoutSkeletonOnly connectionSkeletonOnly(confSkeletonOnly);
    EXPECT_EQ(
        ::middleware::core::HRESULT::Ok, connectionSkeletonOnly.subscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(connectionSkeletonOnly.unsubscribe(skeletonInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeBidirectional)
{
    ProxyMock proxyInstance(1, 2, 4);
    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationBidirectionalMock confBidirectional;

    ClusterConnectionNoTimeoutBidirectional connectionBidirectional(confBidirectional);
    EXPECT_EQ(
        ::middleware::core::HRESULT::Ok, connectionBidirectional.subscribe(skeletonInstance, 1));
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connectionBidirectional.subscribe(proxyInstance, 1));
    EXPECT_NO_THROW(connectionBidirectional.unsubscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(connectionBidirectional.unsubscribe(proxyInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeProxyOnlyWithTimeout)
{
    ProxyMock proxyInstance(1, 2, 4);
    ClusterConnectionConfigurationProxyOnlyTimeoutMock confProxyOnlyTimeout;

    ClusterConnectionProxyOnlyWithTimeout connectionProxyOnly(confProxyOnlyTimeout);
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connectionProxyOnly.subscribe(proxyInstance, 1));
    EXPECT_NO_THROW(connectionProxyOnly.unsubscribe(proxyInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeSkeletonOnlyWithTimeout)
{
    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyTimeoutMock confSkeletonOnlyTimeout;

    ClusterConnectionSkeletonOnlyWithTimeout connectionSkeletonOnly(confSkeletonOnlyTimeout);
    EXPECT_EQ(
        ::middleware::core::HRESULT::Ok, connectionSkeletonOnly.subscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(connectionSkeletonOnly.unsubscribe(skeletonInstance, 1));
}

TEST_F(ConnectionBaseTest, SubscribeUnsubscribeBidirectionalWithTimeout)
{
    ProxyMock proxyInstance(1, 2, 4);
    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationBidirectionalTimeoutMock confBidirectionalTimeout;

    ClusterConnectionBidirectionalWithTimeout connectionBidirectional(confBidirectionalTimeout);
    EXPECT_EQ(
        ::middleware::core::HRESULT::Ok, connectionBidirectional.subscribe(skeletonInstance, 1));
    EXPECT_EQ(::middleware::core::HRESULT::Ok, connectionBidirectional.subscribe(proxyInstance, 1));
    EXPECT_NO_THROW(connectionBidirectional.unsubscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(connectionBidirectional.unsubscribe(proxyInstance, 1));
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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // next invocation of write() on the cluster connection config will return false
    confSkeletonOnly.setNextWriteResult(false);

    logger_mock_.EXPECT_EVENT_LOG(
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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // sendMessage: Setting error code ServiceNotFound exactly one time for the receiving side, but
    // fall back to OK when sending back the error
    confSkeletonOnly.setNextHRESULT(::middleware::core::HRESULT::ServiceNotFound, 1);

    logger_mock_.EXPECT_EVENT_LOG(
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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));

    // sendMessage: Setting error code ServiceNotFound exactly one time for the receiving side, but
    // fall back to OK when sending back the error
    confSkeletonOnly.setNextHRESULT(::middleware::core::HRESULT::ServiceBusy, 1);

    logger_mock_.EXPECT_EVENT_LOG(
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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

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

    SkeletonMock skeletonInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    EXPECT_EQ(::middleware::core::HRESULT::Ok, actualConnection.subscribe(skeletonInstance, 1));
    EXPECT_NO_THROW(ptrToBase->processMessage(msg));

    auto lastReceivedMsg = confSkeletonOnly.getLastReceivedMessage();
    EXPECT_EQ(lastReceivedMsg.value().getErrorState(), ErrorState::NoError);
}

TEST_F(ConnectionBaseTest, ProxyRegistersAsTimeoutTransceiver)
{
    ProxyMockWithTimeout proxyInstance(1, 2);
    ClusterConnectionConfigurationSkeletonOnlyTimeoutMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyTimeoutMock>::type
        actualConnection(confSkeletonOnly);

    EXPECT_NO_THROW(actualConnection.registerTimeoutTransceiver(proxyInstance));
    EXPECT_EQ(1, actualConnection.registeredTransceiversCount(1));
    EXPECT_NO_THROW(actualConnection.updateTimeouts());

    EXPECT_TRUE(confSkeletonOnly.hasBeenTriggered());

    EXPECT_NO_THROW(actualConnection.unregisterTimeoutTransceiver(proxyInstance));
    EXPECT_EQ(0, actualConnection.registeredTransceiversCount(1));
}

TEST_F(ConnectionBaseTest, SyntheticClusterIdGetter)
{
    ClusterConnectionConfigurationSkeletonOnlyMock confSkeletonOnly;
    ::middleware::core::ClusterConnectionTypeSelector<
        ClusterConnectionConfigurationSkeletonOnlyMock>::type actualConnection(confSkeletonOnly);

    // ptr to base class trick as observed in {Proxy/Skeleton}Base
    ::middleware::core::IClusterConnection* ptrToBase = &actualConnection;

    // expectation: Connection getters are just relaying to the configuration getters
    EXPECT_EQ(ptrToBase->getSourceClusterId(), confSkeletonOnly.getSourceClusterId());
    EXPECT_EQ(ptrToBase->getTargetClusterId(), confSkeletonOnly.getTargetClusterId());
}

} // namespace test
} // namespace core
} // namespace middleware
