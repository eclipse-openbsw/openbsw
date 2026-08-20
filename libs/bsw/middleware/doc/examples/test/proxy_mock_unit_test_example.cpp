/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <etl/delegate.h>
#include <etl/optional.h>
#include <gtest/gtest.h>

#include "features/communication/dummy_serviceProxy.h"
#include "features/communication/dummy_serviceProxyMock.h"
#include "gmock/gmock.h"
#include "logger/mock/LoggerMock.h"
#include "memory/mock/AllocatorMock.h"
#include "middleware/core/types.h"
#include "proxy_app.h"

namespace application
{
namespace test
{

class ProxyAppTestFixture : public ::testing::Test
{
public:
    using MockType            = features::communication::DummyService::proxy::DummyServiceProxyMock;
    using Foo                 = ProxyApp::Foo;
    using Baz                 = ProxyApp::Baz;
    using AsyncMethodCallback = ProxyApp::AsyncMethodCallback;
    using AsyncMethodResult   = ProxyApp::AsyncMethodResult;

    ProxyAppTestFixture()                                      = default;
    ~ProxyAppTestFixture() override                            = default;
    ProxyAppTestFixture(ProxyAppTestFixture const&)            = delete;
    ProxyAppTestFixture(ProxyAppTestFixture&&)                 = delete;
    ProxyAppTestFixture& operator=(ProxyAppTestFixture const&) = delete;
    ProxyAppTestFixture& operator=(ProxyAppTestFixture&&)      = delete;

    void SetUp() final
    {
        middleware::memory::test::AllocatorMock::setAllocatorMock(allocatorMock_);
    }

protected:
    testing::NiceMock<middleware::logger::test::mock::LoggerMock> loggerMock_;
    testing::NiceMock<middleware::memory::test::AllocatorMock> allocatorMock_;
    testing::NiceMock<MockType> mock_;
    ProxyApp app_;
};

// [proxy-async-method-test-start]
TEST_F(ProxyAppTestFixture, AsyncMethodForwardsToMock)
{
    // ARRANGE
    AsyncMethodCallback asyncMethodCb;
    Foo input{1U, 2U};
    uint16_t const kExpectedRequestId = 5U;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, asyncMethod(::testing::_, ::testing::_))
        .WillOnce(::testing::DoAll(
            ::testing::SaveArg<1>(&asyncMethodCb),
            ::testing::Return(
                etl::expected<uint16_t, middleware::core::HRESULT>{kExpectedRequestId})));

    // ACT
    app_.startup();
    app_.runAsyncMethod(input);

    // ASSERT
    EXPECT_TRUE(app_.isRequestIdActive());
    EXPECT_EQ(app_.getRequestIdActive(), kExpectedRequestId);

    // ARRANGE
    Baz resultPayload{3U, 4U, 0U, 1U};
    AsyncMethodResult result{etl::reference_wrapper<Baz const>(resultPayload)};

    // ACT
    asyncMethodCb(result); // We trigger our internal callback here.

    // ASSERT
    EXPECT_TRUE(app_.isResponseValid());
    EXPECT_EQ(app_.getResponseValue().a, resultPayload.a);
    EXPECT_EQ(app_.getResponseValue().b, resultPayload.b);
    EXPECT_EQ(app_.getResponseValue().c, resultPayload.c);
    EXPECT_EQ(app_.getResponseValue().d, resultPayload.d);
}

// [proxy-async-method-test-end]

// [proxy-fire-and-forget-test-start]
TEST_F(ProxyAppTestFixture, FireAndForgetMethodForwardsToMock)
{
    // ARRANGE
    ProxyApp::FireAndForgetPayload payload{{1U, 2U}, {3U, 4U}};
    uint16_t const kExpectedRequestId = 5U;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, fireAndForgetMethod(::testing::Ref(payload)))
        .WillOnce(testing::Return(
            etl::expected<uint16_t, middleware::core::HRESULT>{kExpectedRequestId}));

    // ACT
    app_.startup();
    ProxyApp::ProxyApp::MwResult const result = app_.runFireAndForgetMethod(payload);

    // ASSERT
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), kExpectedRequestId);
}

// [proxy-fire-and-forget-test-end]

// [proxy-attribute-get-test-start]
TEST_F(ProxyAppTestFixture, AttributeGetterForwardsToMock)
{
    // ARRANGE
    ProxyApp::AttributeGetterCallback getterCallback;
    uint32_t const attributeValue           = 42U;
    uint16_t const kExpectedGetterRequestId = 6U;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleField, get(::testing::_))
        .WillOnce(::testing::DoAll(
            ::testing::SaveArg<0>(&getterCallback),
            ::testing::Return(
                etl::expected<uint16_t, middleware::core::HRESULT>{kExpectedGetterRequestId})));

    // ACT
    app_.startup();
    ProxyApp::ProxyApp::MwResult const getterResult = app_.runAttributeGet();

    // ASSERT
    ASSERT_TRUE(getterResult.has_value());
    EXPECT_EQ(getterResult.value(), kExpectedGetterRequestId);

    // ARRANGE
    ProxyApp::AttributeGetterResult const attributeResult{
        etl::reference_wrapper<uint32_t const>(attributeValue)};

    // ACT
    getterCallback(attributeResult);

    // ASSERT
    ASSERT_TRUE(attributeResult.has_value());
    EXPECT_EQ(attributeResult.value().get(), attributeValue);
    EXPECT_EQ(app_.receivedAttributeValue(), attributeValue);
}

// [proxy-attribute-get-test-end]

// [proxy-attribute-set-test-start]
TEST_F(ProxyAppTestFixture, AttributeSetterForwardsToMock)
{
    // ARRANGE
    uint16_t const kExpectedSetterRequestId = 7U;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleField, set(42U))
        .WillOnce(testing::Return(
            etl::expected<uint16_t, middleware::core::HRESULT>{kExpectedSetterRequestId}));

    // ACT
    app_.startup();
    ProxyApp::ProxyApp::MwResult const setterResult = app_.runAttributeSet(42U);

    // ASSERT
    ASSERT_TRUE(setterResult.has_value());
    EXPECT_EQ(setterResult.value(), kExpectedSetterRequestId);
}

// [proxy-attribute-set-test-end]

// [proxy-attribute-receive-test-start]
TEST_F(ProxyAppTestFixture, AttributeReceiveForwardsToMock)
{
    // ARRANGE
    ProxyApp::AttributeReceiveCallback attributeCallback;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleField, setReceiveHandler(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&attributeCallback));

    uint32_t const attributeValue = 42U;

    // ACT
    app_.startup();
    attributeCallback(attributeValue);

    // ASSERT
    EXPECT_EQ(app_.receivedAttributeValue(), attributeValue);
}

// [proxy-attribute-receive-test-end]

// [proxy-event-receive-test-start]
TEST_F(ProxyAppTestFixture, EventReceiveForwardsToMock)
{
    // ARRANGE
    ProxyApp::EventReceiveCallback eventCallback;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleBroadcast, setReceiveHandler(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&eventCallback));

    uint32_t const eventValue = 99U;

    // ACT
    app_.startup();
    eventCallback(eventValue);

    // ASSERT
    EXPECT_EQ(app_.receivedEventValue(), eventValue);
}

// [proxy-event-receive-test-end]

} // namespace test
} // namespace application
