#include <etl/array.h>
#include <etl/vector.h>

#include "gtest/gtest.h"
#include "middleware/core/DatabaseManipulator.h"
#include "proxy.h"
#include "skeleton.h"

using ::middleware::core::HRESULT;
using ::middleware::core::ITransceiver;
using ::middleware::core::meta::DbManipulator;
using ::middleware::core::meta::TransceiverContainer;

class DbManipulatorTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        // setup proxies
        (_proxyTransceivers[0]).fContainer->emplace_back(&_proxy1);
        (_proxyTransceivers[0]).fContainer->emplace_back(&_proxy2);
        (_proxyTransceivers[1]).fContainer->emplace_back(&_proxy3);

        etl::sort(
            _proxyTransceivers[0].fContainer->begin(),
            _proxyTransceivers[0].fContainer->end(),
            TransceiverContainer::TransceiverComparator());
        etl::sort(
            _proxyTransceivers[1].fContainer->begin(),
            _proxyTransceivers[1].fContainer->end(),
            TransceiverContainer::TransceiverComparator());

        // setup skeletons
        _skeletonTransceivers[0].fContainer->emplace_back(&_skeleton1);

        etl::sort(
            _skeletonTransceivers[1].fContainer->begin(),
            _skeletonTransceivers[1].fContainer->end(),
            TransceiverContainer::TransceiverComparator());
    }

    void TearDown() override
    {
        // cleanup proxies
        _proxyTransceivers[0].fContainer->clear();
        _proxyTransceivers[1].fContainer->clear();
        // cleanup skeletons
        _skeletonTransceivers[0].fContainer->clear();
        _skeletonTransceivers[1].fContainer->clear();
    }

private:
    etl::vector<ITransceiver*, 3U> _proxyTransceivers41{};
    etl::vector<ITransceiver*, 1U> _proxyTransceivers54{};
    etl::ivector<ITransceiver*>& _iProxyTransceivers41{_proxyTransceivers41};
    etl::ivector<ITransceiver*>& _iProxyTransceivers54{_proxyTransceivers54};

    etl::vector<ITransceiver*, 1U> _skeletonTransceivers41{};
    etl::vector<ITransceiver*, 1U> _skeletonTransceivers54{};
    etl::ivector<ITransceiver*>& _iSkeletonTransceivers41{_skeletonTransceivers41};
    etl::ivector<ITransceiver*>& _iSkeletonTransceivers54{_skeletonTransceivers54};

protected:
    ProxyMock _proxy1{0x41, 0x01, 0x01};
    ProxyMock _proxy2{0x41, 0x01, 0x02};
    ProxyMock _proxy3{0x54, 0x02, 0x03};
    SkeletonMock _skeleton1{0x41, 0x01};

    etl::array<TransceiverContainer, 2U> _proxyTransceivers{
        {{&_iProxyTransceivers41, 0x41, 0U}, {&_iProxyTransceivers54, 0x54, 0U}}};

    etl::array<TransceiverContainer, 2U> _skeletonTransceivers{
        {{&_iSkeletonTransceivers41, 0x41, 0U}, {&_iSkeletonTransceivers54, 0x54, 0U}}};
};

TEST_F(DbManipulatorTest, TestSubscribeNewProxy)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    ProxyMock proxy(0x41, instanceId, 0x04);

    // ACT
    const HRESULT res = DbManipulator::subscribe(
        _proxyTransceivers.begin(),
        _proxyTransceivers.end(),
        proxy,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(res, HRESULT::Ok);
    EXPECT_EQ(proxy.getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestSubscribeNewSkeleton)
{
    // ARRANGE
    uint16_t const instanceId = 0x02;
    SkeletonMock skeleton(0x54, instanceId);

    // ACT
    const HRESULT res = DbManipulator::subscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        skeleton,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(res, HRESULT::Ok);
    EXPECT_EQ(skeleton.getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestSubscribeProxyWithOutOfRangeServiceId)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    ProxyMock proxy(0x40, instanceId, 0x01);

    // ACT
    const HRESULT res = DbManipulator::subscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, instanceId, 0);

    // ASSERT
    EXPECT_EQ(res, HRESULT::ServiceIdOutOfRange);
    EXPECT_EQ(proxy.getInstanceId(), ::middleware::core::INVALID_INSTANCE_ID);
}

TEST_F(DbManipulatorTest, TestSubscribeProxyWithUnknownServiceId)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    ProxyMock proxy(0x40, instanceId, 0x01);

    // ACT
    const HRESULT res = DbManipulator::subscribe(
        _proxyTransceivers.begin(),
        _proxyTransceivers.end(),
        proxy,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(res, HRESULT::ServiceNotFound);
    EXPECT_EQ(proxy.getInstanceId(), ::middleware::core::INVALID_INSTANCE_ID);
}

TEST_F(DbManipulatorTest, TestSubscribeSkeletonWithOutOfRangeServiceId)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    SkeletonMock skeleton(0x40, instanceId);

    // ACT
    const HRESULT res = DbManipulator::subscribe(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), skeleton, instanceId, 0);

    // ASSERT
    EXPECT_EQ(res, HRESULT::ServiceIdOutOfRange);
    EXPECT_EQ(skeleton.getInstanceId(), ::middleware::core::INVALID_INSTANCE_ID);
}

TEST_F(DbManipulatorTest, TestSubscribeSkeletonWithUnknownServiceId)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    SkeletonMock skeleton(0x40, instanceId);

    // ACT
    const HRESULT res = DbManipulator::subscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        skeleton,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(res, HRESULT::ServiceNotFound);
    EXPECT_EQ(skeleton.getInstanceId(), ::middleware::core::INVALID_INSTANCE_ID);
}

TEST_F(DbManipulatorTest, TestSubscribeProxyWithTransceiverAlreadyRegistered)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    ProxyMock proxy(0x41, instanceId, 0x01);

    // ACT
    const HRESULT resProxy = DbManipulator::subscribe(
        _proxyTransceivers.begin(),
        _proxyTransceivers.end(),
        proxy,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(resProxy, HRESULT::Ok);
    EXPECT_EQ(proxy.getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestSubscribeSkeletonWithTransceiverAlreadyRegistered)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    SkeletonMock skeleton(0x41, instanceId);

    // ACT
    const HRESULT resSkeleton = DbManipulator::subscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        skeleton,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(resSkeleton, HRESULT::InstanceAlreadyRegistered);
    EXPECT_EQ(skeleton.getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestProxySubscribeWithFullContainer)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    ProxyMock proxy1(0x41, instanceId, 0x04);
    ProxyMock proxy2(0x41, instanceId, 0x05);

    // ACT
    const HRESULT resProxy1 = DbManipulator::subscribe(
        _proxyTransceivers.begin(),
        _proxyTransceivers.end(),
        proxy1,
        instanceId,
        etl::numeric_limits<uint16_t>::max());
    const HRESULT resProxy2 = DbManipulator::subscribe(
        _proxyTransceivers.begin(),
        _proxyTransceivers.end(),
        proxy2,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(resProxy1, HRESULT::Ok);
    EXPECT_EQ(resProxy2, HRESULT::TransceiverInitializationFailed);
}

TEST_F(DbManipulatorTest, TestSkeletonSubscribeWithFullContainer)
{
    // ARRANGE
    uint16_t const instanceId = 0x02;
    SkeletonMock skeleton(0x41, instanceId);

    // ACT
    const HRESULT resSkeleton = DbManipulator::subscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        skeleton,
        instanceId,
        etl::numeric_limits<uint16_t>::max());

    // ASSERT
    EXPECT_EQ(resSkeleton, HRESULT::TransceiverInitializationFailed);
}

TEST_F(DbManipulatorTest, TestProxyUnsubscribe)
{
    // ARRANGE

    // ACT && ASSERT

    // container 0
    EXPECT_EQ(_proxyTransceivers[0].fContainer->size(), 2U);

    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), _proxy1, _proxy1.getServiceId());
    EXPECT_EQ(_proxyTransceivers[0].fContainer->size(), 1U);

    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), _proxy2, _proxy2.getServiceId());
    EXPECT_EQ(_proxyTransceivers[0].fContainer->size(), 0U);

    // nothing happens, unsubscribe was already done
    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), _proxy1, _proxy1.getServiceId());
    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), _proxy2, _proxy2.getServiceId());
    EXPECT_EQ(_proxyTransceivers[0].fContainer->size(), 0U);

    // container 1
    EXPECT_EQ(_proxyTransceivers[1].fContainer->size(), 1U);
    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), _proxy3, _proxy3.getServiceId());
    EXPECT_EQ(_proxyTransceivers[1].fContainer->size(), 0U);

    // nothing happens, unsubscribe was already done
    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), _proxy3, _proxy3.getServiceId());
    EXPECT_EQ(_proxyTransceivers[1].fContainer->size(), 0U);
}

TEST_F(DbManipulatorTest, TestUnknownProxyUnsubscribe)
{
    // ARRANGE
    uint16_t const instanceId = 0x01;
    uint16_t const addressId  = etl::numeric_limits<uint8_t>::max();
    ProxyMock proxy(0xFF, instanceId, addressId);

    // ACT && ASSERT
    EXPECT_EQ(_proxyTransceivers[0].fContainer->size(), 2U);
    EXPECT_EQ(_proxyTransceivers[1].fContainer->size(), 1U);
    DbManipulator::unsubscribe(
        _proxyTransceivers.begin(),
        _proxyTransceivers.end(),
        proxy,
        proxy.getServiceId()); // nothing happens
    EXPECT_EQ(_proxyTransceivers[0].fContainer->size(), 2U);
    EXPECT_EQ(_proxyTransceivers[1].fContainer->size(), 1U);
}

TEST_F(DbManipulatorTest, TestSkeletonUnsubscribe)
{
    // ARRANGE

    // ACT && ASSERT

    // container 0
    EXPECT_EQ(_skeletonTransceivers[0].fContainer->size(), 1U);
    DbManipulator::unsubscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        _skeleton1,
        _skeleton1.getServiceId());
    EXPECT_EQ(_skeletonTransceivers[0].fContainer->size(), 0U);

    // nothing happens, unsubscribe was already done
    DbManipulator::unsubscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        _skeleton1,
        _skeleton1.getServiceId());
}

TEST_F(DbManipulatorTest, TestUnknownSkeletonUnsubscribe)
{
    // ARRANGE
    uint16_t const service    = etl::numeric_limits<uint16_t>::max();
    uint16_t const instanceId = 0x01;
    SkeletonMock skeleton(service, instanceId);

    // ACT && ASSERT
    EXPECT_EQ(_skeletonTransceivers[0].fContainer->size(), 1U);
    EXPECT_EQ(_skeletonTransceivers[1].fContainer->size(), 0U);
    DbManipulator::unsubscribe(
        _skeletonTransceivers.begin(),
        _skeletonTransceivers.end(),
        skeleton,
        skeleton.getServiceId()); // nothing happens
    EXPECT_EQ(_skeletonTransceivers[0].fContainer->size(), 1U);
    EXPECT_EQ(_skeletonTransceivers[1].fContainer->size(), 0U);
}

TEST_F(DbManipulatorTest, TestSkeletonValidSearchByServiceIdAndInstanceId)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = 0x1U;

    // ACT && ASSERT
    ITransceiver const* const transceiver
        = DbManipulator::getSkeletonByServiceIdAndServiceInstanceId(
            _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId);

    EXPECT_EQ(transceiver->getServiceId(), service);
    EXPECT_EQ(transceiver->getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestSkeletonSearchWithUnkownServiceId)
{
    // ARRANGE
    uint16_t const service
        = etl::numeric_limits<uint16_t>::max(); // unknown service id to the database
    uint16_t const instanceId = 0x1U;

    // ACT && ASSERT
    ITransceiver const* const transceiver
        = DbManipulator::getSkeletonByServiceIdAndServiceInstanceId(
            _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestSkeletonSearchWithUnkownInstanceId)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = etl::numeric_limits<uint16_t>::max();

    // ACT && ASSERT
    ITransceiver const* const transceiver
        = DbManipulator::getSkeletonByServiceIdAndServiceInstanceId(
            _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestProxyTransceiverValidSearch)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = 0x1U;
    uint16_t const addressId  = 0x1U;

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver->getServiceId(), service);
    EXPECT_EQ(transceiver->getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestProxyTranceiverSearchWithUnkownServiceId)
{
    // ARRANGE
    uint16_t const service
        = etl::numeric_limits<uint16_t>::max(); // unknown service id to the database
    uint16_t const instanceId = 0x1U;
    uint16_t const addressId  = 0x1U;

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestProxyTranceiverSearchWithUnkownInstanceId)
{
    // ARRANGE
    uint16_t const service = 0x41U;
    uint16_t const instanceId
        = etl::numeric_limits<uint16_t>::max(); // unknown instance id to the database
    uint16_t const addressId = 0x1U;

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestProxyTranceiverSearchWithUnkownAddressId)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = 0x1U;
    uint16_t const addressId
        = etl::numeric_limits<uint8_t>::max(); // unknown address id to the database

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestSkeletonTransceiverValidSearch)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = 0x1U;
    uint16_t const addressId  = etl::numeric_limits<uint8_t>::max();

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver->getServiceId(), service);
    EXPECT_EQ(transceiver->getInstanceId(), instanceId);
}

TEST_F(DbManipulatorTest, TestSkeletonTranceiverSearchWithUnkownServiceId)
{
    // ARRANGE
    uint16_t const service
        = etl::numeric_limits<uint16_t>::max(); // unknown service id to the database
    uint16_t const instanceId = 0x1U;
    uint16_t const addressId  = etl::numeric_limits<uint8_t>::max();

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestSkeletonTranceiverSearchWithUnkownInstanceId)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = 0xFFFFU; // unknown instance id to the database
    uint16_t const addressId  = etl::numeric_limits<uint8_t>::max();

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestSkeletonTranceiverSearchWithUnkownAdressId)
{
    // ARRANGE
    uint16_t const service    = 0x41U;
    uint16_t const instanceId = 0x1U;
    uint16_t const addressId  = 0x1U; // unknown address id to the database

    // ACT && ASSERT
    ITransceiver const* const transceiver = DbManipulator::getTransceiver(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service, instanceId, addressId);

    EXPECT_EQ(transceiver, nullptr);
}

TEST_F(DbManipulatorTest, TestRegisteredProxyTransceiverCount)
{
    // ARRANGE
    uint16_t const service1 = 0x41U;
    uint16_t const service2 = 0x54U;
    uint16_t const service3 = 0x10U; // unknown service id to the database

    // ACT && ASSERT
    std::size_t const proxyCount1 = DbManipulator::registeredTransceiversCount(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service1);
    std::size_t const proxyCount2 = DbManipulator::registeredTransceiversCount(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service2);
    std::size_t const proxyCount3 = DbManipulator::registeredTransceiversCount(
        _proxyTransceivers.begin(), _proxyTransceivers.end(), service3);

    EXPECT_EQ(proxyCount1, 2U);
    EXPECT_EQ(proxyCount2, 1U);
    EXPECT_EQ(proxyCount3, 0U);
}

TEST_F(DbManipulatorTest, TestRegisteredSkeletonTransceiverCount)
{
    // ARRANGE
    uint16_t const service1 = 0x41U;
    uint16_t const service2 = 0x54U; // know service id, but no instance registered on the database
    uint16_t const service3 = 0x10U; // unknown service id to the database

    // ACT && ASSERT
    std::size_t const skeletonCount1 = DbManipulator::registeredTransceiversCount(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service1);
    std::size_t const skeletonCount2 = DbManipulator::registeredTransceiversCount(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service2);
    std::size_t const skeletonCount3 = DbManipulator::registeredTransceiversCount(
        _skeletonTransceivers.begin(), _skeletonTransceivers.end(), service3);

    EXPECT_EQ(skeletonCount1, 1U);
    EXPECT_EQ(skeletonCount2, 0U);
    EXPECT_EQ(skeletonCount3, 0U);
}
