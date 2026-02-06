#include <stdint.h>

#include <etl/array.h>
#include <etl/limits.h>
#include <etl/optional.h>
#include <etl/span.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
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

struct MiddelwareMessageComparator
{
    using MsgType = ::middleware::core::Message;

    bool checkMsgHeader(MsgType const& other) const
    {
        Message::Header const& msgHeader   = _msg.value().getHeader();
        Message::Header const& otherHeader = other.getHeader();
        return msgHeader.srcClusterId == otherHeader.srcClusterId
               && msgHeader.tgtClusterId == otherHeader.tgtClusterId
               && msgHeader.serviceId == otherHeader.serviceId
               && msgHeader.memberId == otherHeader.memberId
               && msgHeader.serviceInstanceId == otherHeader.serviceInstanceId
               && msgHeader.addressId == otherHeader.addressId
               && msgHeader.requestId == otherHeader.requestId
               && msgHeader.flags == otherHeader.flags && _msg.value().isError() == other.isError()
               && _msg.value().isEvent() == other.isEvent()
               && _msg.value().isRequest() == other.isRequest()
               && _msg.value().isResponse() == other.isResponse()
               && _msg.value().isFireAndForgetRequest() == other.isFireAndForgetRequest();
    }

    void setReturnCode(::middleware::core::HRESULT ret) { _ret = ret; }

    ::middleware::core::HRESULT msgReceived(MsgType const& msg)
    {
        _msg = msg;
        return _ret;
    }

private:
    etl::optional<MsgType> _msg;
    ::middleware::core::HRESULT _ret{::middleware::core::HRESULT::Ok};
};

struct ProxyMockStoredMessage
: public ProxyMock
, MiddelwareMessageComparator
{
    using ProxyMock::ProxyMock;

    ::middleware::core::HRESULT
    onNewMessageReceived(::middleware::core::Message const& msg) override
    {
        return MiddelwareMessageComparator::msgReceived(msg);
    }
};

struct SkeletonMockStoredMessage
: public SkeletonMock
, MiddelwareMessageComparator
{
    using SkeletonMock::SkeletonMock;

    ::middleware::core::HRESULT
    onNewMessageReceived(::middleware::core::Message const& msg) override
    {
        return MiddelwareMessageComparator::msgReceived(msg);
    }
};

struct TimeoutMock : middleware::core::ITimeout
{
    void updateTimeouts() { _triggered = true; }

    bool hasBeenTriggered() { return _triggered; }

private:
    bool _triggered{false};
};

struct ClusterConfigurationNoTimeout : public IClusterConnectionConfigurationBase
{
    static uint16_t const serviceId{12};
    static uint16_t const instanceId{1};
    static uint16_t const addressId{1};
    static uint8_t const sourceClusterId{1};
    static uint8_t const targetClusterId{2};

    ClusterConfigurationNoTimeout()
    {
        // setup proxies
        (_proxyTransceivers[0]).fContainer->emplace_back(&_proxy);

        etl::sort(
            _proxyTransceivers[0].fContainer->begin(),
            _proxyTransceivers[0].fContainer->end(),
            meta::TransceiverContainer::TransceiverComparator());

        // setup skeletons
        _skeletonTransceivers[0].fContainer->emplace_back(&_skeleton);

        etl::sort(
            _skeletonTransceivers[0].fContainer->begin(),
            _skeletonTransceivers[0].fContainer->end(),
            meta::TransceiverContainer::TransceiverComparator());
    }

    uint8_t getSourceClusterId() const override { return sourceClusterId; }

    uint8_t getTargetClusterId() const override { return targetClusterId; }

    bool write(Message const&) const override { return true; }

    std::size_t registeredTransceiversCount(uint16_t const) const override { return 0; }

    HRESULT dispatchMessage(Message const& msg) const override
    {
        return IClusterConnectionConfigurationBase::dispatchMessage(
            std::begin(_proxyTransceivers),
            std::end(_proxyTransceivers),
            std::begin(_skeletonTransceivers),
            std::end(_skeletonTransceivers),
            msg);
    }

    // relaying to protected base class methods
    HRESULT dispatchMessageToProxy(Message const& msg)
    {
        return IClusterConnectionConfigurationBase::dispatchMessageToProxy(
            std::begin(_proxyTransceivers), std::end(_proxyTransceivers), msg);
    }

    // relaying to protected base class methods
    HRESULT dispatchMessageToSkeleton(Message const& msg)
    {
        return IClusterConnectionConfigurationBase::dispatchMessageToSkeleton(
            std::begin(_skeletonTransceivers), std::end(_skeletonTransceivers), msg);
    }

    ProxyMockStoredMessage& getProxy() { return _proxy; }

    SkeletonMockStoredMessage& getSkeleton() { return _skeleton; }

private:
    etl::vector<::middleware::core::ITransceiver*, 1U> _proxyTransceiversAlloc{};
    etl::ivector<::middleware::core::ITransceiver*>& _iProxyTransceivers{_proxyTransceiversAlloc};

    etl::vector<::middleware::core::ITransceiver*, 1U> _skeletonTransceiversAlloc{};
    etl::ivector<::middleware::core::ITransceiver*>& _iSkeletonTransceivers{
        _skeletonTransceiversAlloc};

    ::middleware::core::meta::TransceiverContainer _proxyTransceivers[1]{
        {&_iProxyTransceivers, ClusterConfigurationNoTimeout::serviceId, 0U}};
    ::middleware::core::meta::TransceiverContainer _skeletonTransceivers[1]{
        {&_iSkeletonTransceivers, ClusterConfigurationNoTimeout::serviceId, 0U}};

protected:
    ProxyMockStoredMessage _proxy{
        ClusterConfigurationNoTimeout::serviceId,
        ClusterConfigurationNoTimeout::instanceId,
        ClusterConfigurationNoTimeout::addressId};
    SkeletonMockStoredMessage _skeleton{
        ClusterConfigurationNoTimeout::serviceId, ClusterConfigurationNoTimeout::instanceId};
};

struct ClusterConfigurationTimeout : ITimeoutConfiguration
{
    static size_t const MAX_TIMEOUT_RECEIVERS = 2;

    // implementing ITimeoutConfiguration`
    uint8_t getSourceClusterId() const override { return static_cast<uint8_t>(1); }

    uint8_t getTargetClusterId() const override { return static_cast<uint8_t>(2); }

    bool write(Message const&) const override { return true; }

    std::size_t registeredTransceiversCount(uint16_t const) const override { return 0; }

    HRESULT dispatchMessage(Message const&) const override
    {
        return ::middleware::core::HRESULT::Ok;
    }

    void registerTimeoutTransceiver(ITimeout& transceiver) override
    {
        ITimeoutConfiguration::registerTimeoutTransceiver(transceiver, _timeoutTransceiver);
    }

    void unregisterTimeoutTransceiver(ITimeout& transceiver) override
    {
        ITimeoutConfiguration::unregisterTimeoutTransceiver(transceiver, _timeoutTransceiver);
    }

    void updateTimeouts() override { ITimeoutConfiguration::updateTimeouts(_timeoutTransceiver); }

    // helper test functions accessing the container
    size_t numTransceivers() { return _timeoutTransceiver.size(); }

    bool containsTransceiver(ITimeout const& transceiver)
    {
        return _timeoutTransceiver.cend()
               != etl::find(_timeoutTransceiver.cbegin(), _timeoutTransceiver.cend(), &transceiver);
    }

private:
    etl::vector<::middleware::core::ITimeout*, MAX_TIMEOUT_RECEIVERS> _timeoutTransceiverAlloc{};
    etl::ivector<::middleware::core::ITimeout*>& _timeoutTransceiver{_timeoutTransceiverAlloc};
};

class ConfigurationBaseTest : public ::testing::Test
{
public:
    void SetUp() override {}

    void TearDown() override {}

    static Message createRequestMessage(uint16_t const memberId, uint16_t const requestId)
    {
        Message msg = Message::createRequest(
            ClusterConfigurationNoTimeout::serviceId,
            memberId,
            requestId,
            ClusterConfigurationNoTimeout::instanceId,
            ClusterConfigurationNoTimeout::sourceClusterId,
            ClusterConfigurationNoTimeout::targetClusterId,
            ClusterConfigurationNoTimeout::addressId);
        return msg;
    }

    static Message createInvalidRequestMessage(uint16_t const memberId, uint16_t const requestId)
    {
        Message msg = Message::createRequest(
            ClusterConfigurationNoTimeout::serviceId
                + 1 /* offset ensuring there is no hit in the DB*/,
            memberId,
            requestId,
            ClusterConfigurationNoTimeout::instanceId,
            ClusterConfigurationNoTimeout::sourceClusterId,
            ClusterConfigurationNoTimeout::targetClusterId,
            ClusterConfigurationNoTimeout::addressId
                + 1 /* offset ensuring there is no hit in the DB*/);
        return msg;
    }

    static Message createResponseMessage(uint16_t const memberId, uint16_t const requestId)
    {
        Message msg = Message::createResponse(
            ClusterConfigurationNoTimeout::serviceId,
            memberId,
            requestId,
            ClusterConfigurationNoTimeout::instanceId,
            ClusterConfigurationNoTimeout::sourceClusterId,
            ClusterConfigurationNoTimeout::targetClusterId,
            ClusterConfigurationNoTimeout::addressId);
        return msg;
    }

    static Message createInvalidResponseMessage(uint16_t const memberId, uint16_t const requestId)
    {
        Message msg = Message::createResponse(
            ClusterConfigurationNoTimeout::serviceId
                + 1 /* offset ensuring there is no hit in the DB*/,
            memberId,
            requestId,
            ClusterConfigurationNoTimeout::instanceId,
            ClusterConfigurationNoTimeout::sourceClusterId,
            ClusterConfigurationNoTimeout::targetClusterId,
            ClusterConfigurationNoTimeout::addressId
                + 1 /* offset ensuring there is no hit in the DB*/);
        return msg;
    }

    static Message createEvent(uint16_t const memberId)
    {
        Message msg = Message::createEvent(
            ClusterConfigurationNoTimeout::serviceId,
            memberId,
            ClusterConfigurationNoTimeout::instanceId,
            ClusterConfigurationNoTimeout::sourceClusterId);
        msg.setTargetClusterId(ClusterConfigurationNoTimeout::targetClusterId);

        return msg;
    }

protected:
    ClusterConfigurationNoTimeout _clusterConf;
    ClusterConfigurationTimeout _clusterTimeoutConf;
};

TEST_F(ConfigurationBaseTest, ProxyTargetRouted)
{
    Message prxyTrgtMsg = createResponseMessage(123, 321);

    EXPECT_EQ(::middleware::core::HRESULT::Ok, _clusterConf.dispatchMessage(prxyTrgtMsg));
}

TEST_F(ConfigurationBaseTest, ProxyTargetRoutedEvent)
{
    Message eventMsg = createEvent(123);

    EXPECT_TRUE(eventMsg.isEvent());
    EXPECT_EQ(::middleware::core::HRESULT::Ok, _clusterConf.dispatchMessage(eventMsg));
    EXPECT_TRUE(_clusterConf.getProxy().checkMsgHeader(eventMsg));
}

TEST_F(ConfigurationBaseTest, ProxyTargetRoutedFailed)
{
    Message prxyTrgtMsg = createInvalidResponseMessage(123, 321);

    // expectation: Fall through, returning HRESULT::Ok if adressing params can't be matched to a
    // registered proxy
    EXPECT_EQ(::middleware::core::HRESULT::Ok, _clusterConf.dispatchMessage(prxyTrgtMsg));
}

TEST_F(ConfigurationBaseTest, ProxySkeletonWrongEndpoint)
{
    Message prxyTrgtMsg = createInvalidRequestMessage(123, 321);

    EXPECT_EQ(
        ::middleware::core::HRESULT::RoutingError,
        _clusterConf.dispatchMessageToProxy(prxyTrgtMsg));
}

TEST_F(ConfigurationBaseTest, SkeletonTargetRouted)
{
    Message skltnTrgtMsg = createRequestMessage(123, 321);

    EXPECT_EQ(::middleware::core::HRESULT::Ok, _clusterConf.dispatchMessage(skltnTrgtMsg));
}

TEST_F(ConfigurationBaseTest, SkeletonTargetRoutedMemberNotFound)
{
    Message skltnTrgtMsg = createRequestMessage(123, 321);

    _clusterConf.getSkeleton().setReturnCode(::middleware::core::HRESULT::ServiceMemberIdNotFound);
    EXPECT_EQ(
        ::middleware::core::HRESULT::ServiceMemberIdNotFound,
        _clusterConf.dispatchMessage(skltnTrgtMsg));
}

TEST_F(ConfigurationBaseTest, SkeletonTargetRoutedServiceBusy)
{
    Message skltnTrgtMsg = createRequestMessage(123, 321);

    _clusterConf.getSkeleton().setReturnCode(::middleware::core::HRESULT::ServiceBusy);
    EXPECT_EQ(::middleware::core::HRESULT::ServiceBusy, _clusterConf.dispatchMessage(skltnTrgtMsg));
}

TEST_F(ConfigurationBaseTest, SkeletonTargetRoutedArbitraryReturn)
{
    Message skltnTrgtMsg = createRequestMessage(123, 321);

    // setting any return code not created by the framework but the receiver. Expecting pass
    // through.
    _clusterConf.getSkeleton().setReturnCode(::middleware::core::HRESULT::NotImplemented);
    EXPECT_EQ(
        ::middleware::core::HRESULT::NotImplemented, _clusterConf.dispatchMessage(skltnTrgtMsg));
}

TEST_F(ConfigurationBaseTest, SkeletonTargetRoutedFailed)
{
    Message skltnTrgtMsg = createInvalidRequestMessage(123, 321);

    EXPECT_EQ(
        ::middleware::core::HRESULT::ServiceNotFound, _clusterConf.dispatchMessage(skltnTrgtMsg));
}

TEST_F(ConfigurationBaseTest, SkeletonProxyWrongEndpoint)
{
    Message skltnTrgtMsg = createResponseMessage(123, 321);

    EXPECT_EQ(
        ::middleware::core::HRESULT::RoutingError,
        _clusterConf.dispatchMessageToSkeleton(skltnTrgtMsg));
}

TEST_F(ConfigurationBaseTest, TimeoutTransceiverAddRemove)
{
    TimeoutMock rec1;
    TimeoutMock rec2;

    EXPECT_EQ(0, _clusterTimeoutConf.numTransceivers());

    // add first receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec1);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));

    // add second receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec2);
    EXPECT_EQ(2, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec2));

    // delete second receiver
    _clusterTimeoutConf.unregisterTimeoutTransceiver(rec2);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));

    // delete first receiver
    _clusterTimeoutConf.unregisterTimeoutTransceiver(rec1);
    EXPECT_EQ(0, _clusterTimeoutConf.numTransceivers());
}

TEST_F(ConfigurationBaseTest, TimeoutTransceiverAddButFull)
{
    TimeoutMock rec1;
    TimeoutMock rec2;
    TimeoutMock rec3;

    EXPECT_EQ(0, _clusterTimeoutConf.numTransceivers());

    // add first receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec1);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));

    // add second receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec2);
    EXPECT_EQ(2, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec2));

    // (try) add third receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec3);
    EXPECT_EQ(2, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec2));

    EXPECT_FALSE(_clusterTimeoutConf.containsTransceiver(rec3));
}

TEST_F(ConfigurationBaseTest, TimeoutTransceiverDoubleDelete)
{
    TimeoutMock rec1;
    TimeoutMock rec2;

    EXPECT_EQ(0, _clusterTimeoutConf.numTransceivers());

    // add first receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec1);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));

    // add second receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec2);
    EXPECT_EQ(2, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec2));

    // delete second receiver
    _clusterTimeoutConf.unregisterTimeoutTransceiver(rec2);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_FALSE(_clusterTimeoutConf.containsTransceiver(rec2));

    // delete second receiver again
    _clusterTimeoutConf.unregisterTimeoutTransceiver(rec2);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_FALSE(_clusterTimeoutConf.containsTransceiver(rec2));
}

TEST_F(ConfigurationBaseTest, TimeoutTransceiverDoubleInsert)
{
    TimeoutMock rec1;

    EXPECT_EQ(0, _clusterTimeoutConf.numTransceivers());

    // add first receiver
    _clusterTimeoutConf.registerTimeoutTransceiver(rec1);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));

    // add first receiver again
    _clusterTimeoutConf.registerTimeoutTransceiver(rec1);
    EXPECT_EQ(1, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
}

TEST_F(ConfigurationBaseTest, TimeoutTransceiverTriggerTest)
{
    TimeoutMock rec1;
    TimeoutMock rec2;

    // add receivers
    _clusterTimeoutConf.registerTimeoutTransceiver(rec1);
    _clusterTimeoutConf.registerTimeoutTransceiver(rec2);
    EXPECT_EQ(2, _clusterTimeoutConf.numTransceivers());
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec1));
    EXPECT_TRUE(_clusterTimeoutConf.containsTransceiver(rec2));

    // trigger all receivers
    _clusterTimeoutConf.updateTimeouts();

    EXPECT_TRUE(rec1.hasBeenTriggered());
    EXPECT_TRUE(rec2.hasBeenTriggered());
}

} // namespace test
} // namespace core
} // namespace middleware
