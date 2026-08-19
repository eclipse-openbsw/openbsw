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

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/functional.h>
#include <etl/type_traits.h>
#include <etl/utility.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/mock/ProxyMock.h"
#include "logger/DslLogger.h"
#include "memory/mock/AllocatorMock.h"
#include "middleware/core/Future.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/rpc/ProxyMethod.h"
#include "time/mock/SystemTimerProviderMock.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

namespace middleware::rpc::test
{

// ---------------------------------------------------------------------------
// ProxyMethod tests
// ---------------------------------------------------------------------------

static constexpr uint16_t TEST_METHOD_ID = 42U;

/// A payload type larger than Message::MAX_PAYLOAD_SIZE to force external allocation.
struct LargePayload
{
    etl::array<uint8_t, core::Message::MAX_PAYLOAD_SIZE + 16U> data{};

    bool operator==(LargePayload const& other) const { return data == other.data; }
};

template<typename DataType, uint32_t TIMEOUT, uint8_t LIMIT>
struct TestConfig
{
    using InputType                         = DataType;
    using Traits                            = MethodTraits<DataType, DataType, TIMEOUT>;
    static constexpr uint8_t REQUEST_LIMIT  = LIMIT;
    static constexpr uint32_t TIMEOUT_VALUE = TIMEOUT;
    static constexpr bool USES_EXTERNAL_ALLOCATION
        = sizeof(DataType) > core::Message::MAX_PAYLOAD_SIZE;
};

template<typename Config>
class ProxyMethodTestBase : public ::testing::Test
{
public:
    using MethodUnderTest = ProxyMethod<typename Config::Traits, Config::REQUEST_LIMIT>;
    using Result          = typename MethodUnderTest::Result;
    using Callback        = typename MethodUnderTest::Callback;

    class RefApp
    {
    public:
        MOCK_METHOD(void, receiveFutureData, (Result const&));
    };

    static void noopCallbackFn(Result const&) {}

    static Callback noopCallback() { return Callback::template create<&noopCallbackFn>(); }

    void SetUp() override
    {
        time::test::setSystemTimerProviderMock(&_timerMock);
        ON_CALL(_timerMock, getCurrentTimeInMs)
            .WillByDefault([this] { return this->_timerCounter++; });
        ON_CALL(_proxyMock, isInitialized()).WillByDefault(Return(true));
        ON_CALL(_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::Ok));

        memory::test::AllocatorMock::setAllocatorMock(_allocatorMock);

        _loggerMock.setup();
    }

    void TearDown() override
    {
        _method.freeAll();
        time::test::unsetSystemTimerProviderMock();
        _loggerMock.teardown();
    }

protected:
    uint32_t _timerCounter{};
    NiceMock<time::test::SystemTimerProviderMock> _timerMock{};
    middleware::logger::test::DslLogger _loggerMock{};
    NiceMock<core::test::ProxyMock> _proxyMock{};
    NiceMock<memory::test::AllocatorMock> _allocatorMock{};
    MethodUnderTest _method{};
};

static constexpr uint32_t TIMEOUT_OF_ZERO     = 0U;
static constexpr uint32_t TIMEOUT_OF_TEN      = 10U;
static constexpr uint8_t REQUEST_LIMIT_OF_ONE = 1U;
static constexpr uint8_t REQUEST_LIMIT_OF_TEN = 10U;

// ---------------------------------------------------------------------------
// ProxyMethodTestSuite — runs for all configs
// ---------------------------------------------------------------------------

template<typename Config>
class ProxyMethodTestSuite : public ProxyMethodTestBase<Config>
{};

using AllConfigs = ::testing::Types<
    TestConfig<uint32_t, TIMEOUT_OF_ZERO, REQUEST_LIMIT_OF_ONE>,
    TestConfig<uint32_t, TIMEOUT_OF_ZERO, REQUEST_LIMIT_OF_TEN>,
    TestConfig<uint32_t, TIMEOUT_OF_TEN, REQUEST_LIMIT_OF_ONE>,
    TestConfig<uint32_t, TIMEOUT_OF_TEN, REQUEST_LIMIT_OF_TEN>,
    TestConfig<LargePayload, TIMEOUT_OF_ZERO, REQUEST_LIMIT_OF_TEN>>;
TYPED_TEST_SUITE(ProxyMethodTestSuite, AllConfigs);

// --- callMethod / obtainFuture tests ---

TYPED_TEST(ProxyMethodTestSuite, CallMethodReturnsFirstRequestIdZero)
{
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0U);
}

TYPED_TEST(ProxyMethodTestSuite, CallMethodReturnsIncrementingRequestIds)
{
    auto const result1 = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    // The cancel below is needed to in order to free the future for methods with REQUEST_LIMIT=1,
    // otherwise the second call will fail with RequestPoolDepleted
    this->_method.cancelRequest(result1.value());
    auto const result2 = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result1.value(), 0U);
    EXPECT_EQ(result2.value(), 1U);
}

TYPED_TEST(ProxyMethodTestSuite, CallMethodFailsWhenPoolExhausted)
{
    static constexpr uint8_t REQ_LIMIT = TypeParam::REQUEST_LIMIT;
    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        ASSERT_TRUE(this->_method
                        .callMethod(
                            this->_proxyMock,
                            typename TypeParam::InputType{},
                            TEST_METHOD_ID,
                            this->noopCallback())
                        .has_value());
    }
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::RequestPoolDepleted);
}

TYPED_TEST(ProxyMethodTestSuite, CallMethodFailsWhenProxyNotInitialized)
{
    ON_CALL(this->_proxyMock, isInitialized()).WillByDefault(Return(false));
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::NotRegistered);
}

TYPED_TEST(ProxyMethodTestSuite, CallMethodFailsWhenSendFails)
{
    ON_CALL(this->_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::QueueFull));
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::QueueFull);
}

// --- answerRequest / futureMatchingRequestId tests ---

TYPED_TEST(ProxyMethodTestSuite, AnswerRequestInvokesCallbackWithResult)
{
    using RefApp = typename TestFixture::RefApp;
    using Result = typename TestFixture::Result;

    NiceMock<RefApp> appMock{};
    auto cbk = TestFixture::Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, cbk);
    ASSERT_TRUE(reqId.has_value());

    EXPECT_CALL(
        appMock, receiveFutureData(testing::Truly([](Result const& r) { return r.has_value(); })))
        .Times(1U);

    core::Message msg = core::Message::createResponse(0U, 0U, reqId.value(), 0U, 0U, 0U, 0U);
    this->_method.answerRequest(msg);
}

TYPED_TEST(ProxyMethodTestSuite, AnswerRequestInvokesCallbackWithErrorOnErrorMessage)
{
    using RefApp = typename TestFixture::RefApp;
    using Result = typename TestFixture::Result;

    NiceMock<RefApp> appMock{};
    auto cbk = TestFixture::Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, cbk);
    ASSERT_TRUE(reqId.has_value());

    EXPECT_CALL(
        appMock,
        receiveFutureData(testing::Truly(
            [](Result const& r)
            { return !r.has_value() && r.error() == core::Future::State::UserError; })))
        .Times(1U);

    auto msg = core::Message::createErrorResponse(
        0U, 0U, reqId.value(), 0U, 0U, 0U, 0U, core::ErrorState::UserDefinedError);
    this->_method.answerRequest(msg);
}

TYPED_TEST(ProxyMethodTestSuite, AnswerRequestWithMultipleActiveRequests)
{
    static constexpr uint8_t REQ_LIMIT = TypeParam::REQUEST_LIMIT;
    etl::array<uint16_t, REQ_LIMIT> requestIds{};

    auto countingCallback
        = [](typename TestFixture::Result const& r) { EXPECT_TRUE(r.has_value()); };

    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        auto const result = this->_method.callMethod(
            this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, countingCallback);
        ASSERT_TRUE(result.has_value());
        requestIds[i] = result.value();
    }

    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        core::Message msg = core::Message::createResponse(0U, 0U, requestIds[i], 0U, 0U, 0U, 0U);
        this->_method.answerRequest(msg);
    }
}

TYPED_TEST(ProxyMethodTestSuite, AnswerRequestWithInvalidRequestIdLogsError)
{
    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(reqId.has_value());

    core::Message msg
        = core::Message::createResponse(0U, 0U, core::INVALID_REQUEST_ID, 0U, 0U, 0U, 0U);

    this->_loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::SendMessage,
        core::HRESULT::FutureNotFound,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    this->_method.answerRequest(msg);
}

TYPED_TEST(ProxyMethodTestSuite, AnswerRequestWithErrorStatesMapCorrectly)
{
    auto expectedValues = ::etl::make_array<::etl::pair<core::ErrorState, core::Future::State>>(
        ::etl::make_pair(core::ErrorState::UserDefinedError, core::Future::State::UserError),
        ::etl::make_pair(core::ErrorState::ServiceBusy, core::Future::State::ServiceBusy),
        ::etl::make_pair(core::ErrorState::ServiceNotFound, core::Future::State::ServiceNotFound),
        ::etl::make_pair(
            core::ErrorState::SerializationError, core::Future::State::SerializationError),
        ::etl::make_pair(
            core::ErrorState::DeserializationError, core::Future::State::DeserializationError),
        ::etl::make_pair(
            core::ErrorState::QueueFullError, core::Future::State::CouldNotDeliverError));

    using RefApp = typename TestFixture::RefApp;
    using Result = typename TestFixture::Result;

    for (auto& pair : expectedValues)
    {
        NiceMock<RefApp> appMock{};
        auto cbk
            = TestFixture::Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

        auto const reqId = this->_method.callMethod(
            this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, cbk);
        ASSERT_TRUE(reqId.has_value());

        EXPECT_CALL(
            appMock,
            receiveFutureData(testing::Truly(
                [&pair](Result const& r) { return !r.has_value() && r.error() == pair.second; })))
            .Times(1U);

        auto msg
            = core::Message::createErrorResponse(0U, 0U, reqId.value(), 0U, 0U, 0U, 0U, pair.first);
        this->_method.answerRequest(msg);
        this->_method.freeAll();
    }
}

// --- cancelRequest tests ---

TYPED_TEST(ProxyMethodTestSuite, CancelRequestForActiveRequest)
{
    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(reqId.has_value());

    core::HRESULT const result = this->_method.cancelRequest(reqId.value());
    EXPECT_EQ(result, core::HRESULT::Ok);

    // Answering a cancelled request should log an error (future not found)
    core::Message msg = core::Message::createResponse(0U, 0U, reqId.value(), 0U, 0U, 0U, 0U);

    this->_loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::SendMessage,
        core::HRESULT::FutureNotFound,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    this->_method.answerRequest(msg);
}

TYPED_TEST(ProxyMethodTestSuite, CancelRequestWithInvalidRequestId)
{
    uint16_t const invalidId   = 99U;
    core::HRESULT const result = this->_method.cancelRequest(invalidId);
    EXPECT_EQ(result, core::HRESULT::InstanceNotFound);
}

// --- freeAll tests ---

TYPED_TEST(ProxyMethodTestSuite, FreeAllResetsPool)
{
    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(reqId.has_value());

    this->_method.freeAll();

    // After freeAll, we can obtain a new future starting from requestId 0 again
    // (requestId counter is also reset)
    auto const newReqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(newReqId.has_value());
    EXPECT_EQ(newReqId.value(), 0U);
}

TYPED_TEST(ProxyMethodTestSuite, FreeAllWhenSeveralRequestsAreActive)
{
    static constexpr uint8_t REQ_LIMIT = TypeParam::REQUEST_LIMIT;
    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        ASSERT_TRUE(this->_method
                        .callMethod(
                            this->_proxyMock,
                            typename TypeParam::InputType{},
                            TEST_METHOD_ID,
                            this->noopCallback())
                        .has_value());
    }

    this->_method.freeAll();

    // Should be able to fill the pool again
    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        EXPECT_TRUE(this->_method
                        .callMethod(
                            this->_proxyMock,
                            typename TypeParam::InputType{},
                            TEST_METHOD_ID,
                            this->noopCallback())
                        .has_value());
    }
}

// --- RequestId wraparound test ---

TYPED_TEST(ProxyMethodTestSuite, RequestIdWraparound)
{
    for (uint32_t i = 0U; i <= core::INVALID_REQUEST_ID; ++i)
    {
        uint16_t const expectedRequestId = static_cast<uint16_t>(i % core::INVALID_REQUEST_ID);
        auto const reqId                 = this->_method.callMethod(
            this->_proxyMock,
            typename TypeParam::InputType{},
            TEST_METHOD_ID,
            this->noopCallback());
        ASSERT_TRUE(reqId.has_value());
        EXPECT_EQ(reqId.value(), expectedRequestId);
        this->_method.cancelRequest(reqId.value());
    }
}

// ---------------------------------------------------------------------------
// ProxyMethodTimeoutSuite — runs only for configs with TIMEOUT > 0
// ---------------------------------------------------------------------------

template<typename Config>
class ProxyMethodTimeoutSuite : public ProxyMethodTestBase<Config>
{};

using TimeoutConfigs = ::testing::Types<
    TestConfig<uint32_t, TIMEOUT_OF_TEN, REQUEST_LIMIT_OF_ONE>,
    TestConfig<uint32_t, TIMEOUT_OF_TEN, REQUEST_LIMIT_OF_TEN>>;
TYPED_TEST_SUITE(ProxyMethodTimeoutSuite, TimeoutConfigs);

// --- updateTimeouts tests ---

TYPED_TEST(ProxyMethodTimeoutSuite, TimeoutInvokesCallbackWithTimeoutError)
{
    using RefApp = typename TestFixture::RefApp;
    using Result = typename TestFixture::Result;

    NiceMock<RefApp> appMock{};
    auto cbk = TestFixture::Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, cbk);
    ASSERT_TRUE(reqId.has_value());

    EXPECT_CALL(
        appMock,
        receiveFutureData(testing::Truly(
            [](Result const& r)
            { return !r.has_value() && r.error() == core::Future::State::Timeout; })))
        .Times(1U);

    for (uint32_t t = 0U; t <= TypeParam::TIMEOUT_VALUE; ++t)
    {
        this->_method.updateTimeouts();
    }
}

TYPED_TEST(ProxyMethodTimeoutSuite, NoTimeoutBeforeDeadline)
{
    using RefApp = typename TestFixture::RefApp;

    NiceMock<RefApp> appMock{};
    auto cbk = TestFixture::Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    auto const reqId = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, cbk);
    ASSERT_TRUE(reqId.has_value());

    EXPECT_CALL(appMock, receiveFutureData(_)).Times(0U);

    // Call updateTimeouts fewer times than needed to trigger timeout
    for (uint32_t t = 0U; t < TypeParam::TIMEOUT_VALUE; ++t)
    {
        this->_method.updateTimeouts();
    }
}

// ---------------------------------------------------------------------------
// ProxyMethodExternalAllocSuite — runs only for large payload configs
// ---------------------------------------------------------------------------

template<typename Config>
class ProxyMethodExternalAllocSuite : public ProxyMethodTestBase<Config>
{};

using ExternalAllocConfigs
    = ::testing::Types<TestConfig<LargePayload, TIMEOUT_OF_ZERO, REQUEST_LIMIT_OF_TEN>>;
TYPED_TEST_SUITE(ProxyMethodExternalAllocSuite, ExternalAllocConfigs);

// --- External allocation tests ---

TYPED_TEST(ProxyMethodExternalAllocSuite, CallMethodFailsWhenAllocatorReturnsNull)
{
    ON_CALL(this->_allocatorMock, allocateImpl).WillByDefault(Return(nullptr));

    using RefApp = typename TestFixture::RefApp;
    NiceMock<RefApp> appMock{};
    auto cbk = TestFixture::Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    EXPECT_CALL(appMock, receiveFutureData(_)).Times(0U);
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, cbk);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::CannotAllocatePayload);
}

TYPED_TEST(ProxyMethodExternalAllocSuite, AllocationFailureFreesFutureAndAllowsNewRequest)
{
    // First call: allocation fails
    ON_CALL(this->_allocatorMock, allocateImpl).WillByDefault(Return(nullptr));
    auto const failedResult = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_FALSE(failedResult.has_value());
    EXPECT_EQ(failedResult.error(), core::HRESULT::CannotAllocatePayload);

    // Restore allocator defaults to succeed
    memory::test::AllocatorMock::resetAllocatorMockBehaviour(this->_allocatorMock);

    // Second call should succeed, proving the future was freed on allocation failure
    auto const successResult = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, TEST_METHOD_ID, this->noopCallback());
    ASSERT_TRUE(successResult.has_value());
}

TYPED_TEST(ProxyMethodExternalAllocSuite, AllocationFailureDoesNotExhaustRequestPool)
{
    ON_CALL(this->_allocatorMock, allocateImpl).WillByDefault(Return(nullptr));

    // All slots should remain available after allocation failures
    static constexpr uint8_t REQ_LIMIT = TypeParam::REQUEST_LIMIT;
    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        auto const r = this->_method.callMethod(
            this->_proxyMock,
            typename TypeParam::InputType{},
            TEST_METHOD_ID,
            this->noopCallback());
        EXPECT_FALSE(r.has_value());
        EXPECT_EQ(r.error(), core::HRESULT::CannotAllocatePayload);
    }

    // Restore allocator defaults - pool should still be fully available
    memory::test::AllocatorMock::resetAllocatorMockBehaviour(this->_allocatorMock);

    for (uint8_t i = 0U; i < REQ_LIMIT; ++i)
    {
        EXPECT_TRUE(this->_method
                        .callMethod(
                            this->_proxyMock,
                            typename TypeParam::InputType{},
                            TEST_METHOD_ID,
                            this->noopCallback())
                        .has_value());
    }
}

// ---------------------------------------------------------------------------
// ProxyMethodVoidInputSuite — tests for void InputType
// ---------------------------------------------------------------------------

class ProxyMethodVoidInputSuite : public ::testing::Test
{
public:
    using VoidTraits      = MethodTraits<void, void, 0U>;
    using MethodUnderTest = ProxyMethod<VoidTraits, 2U>;
    using Result          = MethodUnderTest::Result;
    using Callback        = MethodUnderTest::Callback;

    class RefApp
    {
    public:
        MOCK_METHOD(void, receiveFutureData, (Result const&));
    };

    static void noopCallbackFn(Result const&) {}

    static Callback noopCallback() { return Callback::template create<&noopCallbackFn>(); }

    void SetUp() override
    {
        time::test::setSystemTimerProviderMock(&_timerMock);
        ON_CALL(_timerMock, getCurrentTimeInMs)
            .WillByDefault([this] { return this->_timerCounter++; });
        ON_CALL(_proxyMock, isInitialized()).WillByDefault(Return(true));
        ON_CALL(_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::Ok));
        _loggerMock.setup();
    }

    void TearDown() override
    {
        _method.freeAll();
        time::test::unsetSystemTimerProviderMock();
        _loggerMock.teardown();
    }

protected:
    uint32_t _timerCounter{};
    NiceMock<time::test::SystemTimerProviderMock> _timerMock{};
    middleware::logger::test::DslLogger _loggerMock{};
    NiceMock<core::test::ProxyMock> _proxyMock{};
    MethodUnderTest _method{};
};

TEST_F(ProxyMethodVoidInputSuite, CallMethodSucceeds)
{
    auto const result = _method.callMethod(_proxyMock, TEST_METHOD_ID, noopCallback());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0U);
}

TEST_F(ProxyMethodVoidInputSuite, CallMethodFailsWhenProxyNotInitialized)
{
    ON_CALL(_proxyMock, isInitialized()).WillByDefault(Return(false));
    auto const result = _method.callMethod(_proxyMock, TEST_METHOD_ID, noopCallback());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::NotRegistered);
}

TEST_F(ProxyMethodVoidInputSuite, CallMethodFailsWhenSendFails)
{
    ON_CALL(_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::QueueFull));
    auto const result = _method.callMethod(_proxyMock, TEST_METHOD_ID, noopCallback());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::QueueFull);
}

TEST_F(ProxyMethodVoidInputSuite, CallMethodFailsWhenPoolExhausted)
{
    ASSERT_TRUE(_method.callMethod(_proxyMock, TEST_METHOD_ID, noopCallback()).has_value());
    ASSERT_TRUE(_method.callMethod(_proxyMock, TEST_METHOD_ID, noopCallback()).has_value());
    auto const result = _method.callMethod(_proxyMock, TEST_METHOD_ID, noopCallback());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::RequestPoolDepleted);
}

TEST_F(ProxyMethodVoidInputSuite, AnswerRequestInvokesCallbackWithResult)
{
    NiceMock<RefApp> appMock{};
    auto cbk = Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    auto const reqId = _method.callMethod(_proxyMock, TEST_METHOD_ID, cbk);
    ASSERT_TRUE(reqId.has_value());

    EXPECT_CALL(
        appMock, receiveFutureData(testing::Truly([](Result const& r) { return r.has_value(); })))
        .Times(1U);

    core::Message msg = core::Message::createResponse(0U, 0U, reqId.value(), 0U, 0U, 0U, 0U);
    _method.answerRequest(msg);
}

TEST_F(ProxyMethodVoidInputSuite, AnswerRequestInvokesCallbackWithErrorOnErrorMessage)
{
    NiceMock<RefApp> appMock{};
    auto cbk = Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

    auto const reqId = _method.callMethod(_proxyMock, TEST_METHOD_ID, cbk);
    ASSERT_TRUE(reqId.has_value());

    EXPECT_CALL(
        appMock,
        receiveFutureData(testing::Truly(
            [](Result const& r)
            { return !r.has_value() && r.error() == core::Future::State::UserError; })))
        .Times(1U);

    auto msg = core::Message::createErrorResponse(
        0U, 0U, reqId.value(), 0U, 0U, 0U, 0U, core::ErrorState::UserDefinedError);
    _method.answerRequest(msg);
}

} // namespace middleware::rpc::test
