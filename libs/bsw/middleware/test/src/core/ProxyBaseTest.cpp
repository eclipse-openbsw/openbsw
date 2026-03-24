#include <cstdint>
#include <stdlib.h>

#include <etl/span.h>

#include "InstancesDatabase.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger/DslLogger.h"
#include "middleware/core/IClusterConnection.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

using testing::Exactly;
using testing::NiceMock;

namespace middleware
{
namespace core
{
namespace test
{

class Proxy : public ProxyBase
{
public:
    HRESULT init(uint16_t instanceId, uint8_t clusterId)
    {
        return ProxyBase::initFromInstancesDatabase(
            instanceId, clusterId, etl::span<IInstanceDatabase const* const>(INSTANCESDATABASE));
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
        HRESULT::TransceiverInitializationFailed,
        kValidclustid,
        proxy.getServiceId(),
        kInvalidinstanceid);

    // ACT & ASSERT
    const HRESULT res = proxy.init(kInvalidinstanceid, kValidclustid);

    EXPECT_EQ(res, HRESULT::TransceiverInitializationFailed);
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

} // namespace test
} // namespace core
} // namespace middleware
