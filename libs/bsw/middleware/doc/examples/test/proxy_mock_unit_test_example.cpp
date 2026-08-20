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

namespace App
{

class App
{
public:
    using Foo                  = features::communication::DummyService::Foo;
    using Baz                  = features::communication::DummyService::Baz;
    using ProxyType            = features::communication::DummyService::proxy::DummyServiceProxy;
    using AsyncMethodResult    = ProxyType::AsyncMethodResult;
    using AsyncMethodCallback  = ProxyType::AsyncMethodCallback;
    using FireAndForgetPayload = features::communication::DummyService::FireAndForgetPayload;
    using AttributeGetterCallback
        = features::communication::DummyService::proxy::SimpleFieldAttribute::GetterCallback;
    using AttributeGetterResult
        = features::communication::DummyService::proxy::SimpleFieldAttribute::GetterResult;
    using AttributeReceiveCallback = features::communication::DummyService::proxy::
        SimpleFieldAttribute::OnFieldChangedCallback;
    using EventReceiveCallback = features::communication::DummyService::proxy::
        SimpleBroadcastEvent::OnFieldChangedCallback;
    using MwResult = etl::expected<uint16_t, middleware::core::HRESULT>;

    bool init()
    {
        middleware::core::HRESULT res = proxy_.init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1);
        return res == middleware::core::HRESULT::Ok;
    }

    // [proxy-app-async-method-start]
    void runAsyncMethod(Foo const& input)
    {
        MwResult res
            = proxy_.asyncMethod(input, etl::make_delegate<App, &App::asyncMethodResponse_>(*this));
        if (res.has_value())
        {
            currentActiveRequestId_ = res.value();
        }
        else
        {
            currentActiveRequestId_ = etl::nullopt;
        }
    }

    // [proxy-app-async-method-end]

    // [proxy-app-fire-and-forget-start]
    MwResult runFireAndForgetMethod(FireAndForgetPayload const& payload)
    {
        return proxy_.fireAndForgetMethod(payload);
    }

    // [proxy-app-fire-and-forget-end]

    // [proxy-app-attribute-get-start]
    MwResult runAttributeGet()
    {
        return proxy_.simpleField.get(
            etl::make_delegate<App, &App::attributeGetterResponse>(*this));
    }

    // [proxy-app-attribute-get-end]

    // [proxy-app-attribute-set-start]
    MwResult runAttributeSet(uint32_t const value) { return proxy_.simpleField.set(value); }

    // [proxy-app-attribute-set-end]

    // [proxy-app-attribute-receive-start]
    void setAttributeReceiveHandler()
    {
        proxy_.simpleField.setReceiveHandler(
            etl::make_delegate<App, &App::attributeChanged>(*this));
    }

    // [proxy-app-attribute-receive-end]

    // [proxy-app-event-receive-start]
    void setEventReceiveHandler()
    {
        proxy_.simpleBroadcast.setReceiveHandler(
            etl::make_delegate<App, &App::eventReceived>(*this));
    }

    // [proxy-app-event-receive-end]

    [[nodiscard]] uint32_t receivedAttributeValue() const { return receivedAttributeValue_; }

    [[nodiscard]] uint32_t receivedEventValue() const { return receivedEventValue_; }

    [[nodiscard]] bool isRequestIdActive() const { return currentActiveRequestId_.has_value(); }

    [[nodiscard]] uint16_t getRequestIdActive() const { return currentActiveRequestId_.value(); }

    [[nodiscard]] bool isResponseValid() const { return latestResult_.has_value(); }

    [[nodiscard]] Baz getResponseValue() const { return latestResult_.value(); }

private:
    void asyncMethodResponse_(AsyncMethodResult const& result)
    {
        if (result.has_value())
        {
            latestResult_ = result.value().get();
        }
        else
        {
            latestResult_ = etl::nullopt;
        }
        currentActiveRequestId_ = etl::nullopt;
    }

    void attributeGetterResponse(AttributeGetterResult const& result)
    {
        if (result.has_value())
        {
            receivedAttributeValue_ = result.value().get();
        }
    }

    void attributeChanged(uint32_t const& value) { receivedAttributeValue_ = value; }

    void eventReceived(uint32_t const& value) { receivedEventValue_ = value; }

    ProxyType proxy_;
    etl::optional<uint16_t> currentActiveRequestId_;
    etl::optional<Baz> latestResult_;
    uint32_t receivedAttributeValue_{};
    uint32_t receivedEventValue_{};
};

namespace Test
{

class ProxyAbstractAppTestFixture : public ::testing::Test
{
public:
    using MockType            = features::communication::DummyService::proxy::DummyServiceProxyMock;
    using Foo                 = App::Foo;
    using Baz                 = App::Baz;
    using AsyncMethodCallback = App::AsyncMethodCallback;
    using AsyncMethodResult   = App::AsyncMethodResult;

    ProxyAbstractAppTestFixture()                                              = default;
    ~ProxyAbstractAppTestFixture() override                                    = default;
    ProxyAbstractAppTestFixture(ProxyAbstractAppTestFixture const&)            = delete;
    ProxyAbstractAppTestFixture(ProxyAbstractAppTestFixture&&)                 = delete;
    ProxyAbstractAppTestFixture& operator=(ProxyAbstractAppTestFixture const&) = delete;
    ProxyAbstractAppTestFixture& operator=(ProxyAbstractAppTestFixture&&)      = delete;

    void SetUp() final
    {
        middleware::memory::test::AllocatorMock::setAllocatorMock(allocatorMock_);
    }

protected:
    testing::NiceMock<middleware::logger::test::mock::LoggerMock> loggerMock_;
    testing::NiceMock<middleware::memory::test::AllocatorMock> allocatorMock_;
    testing::NiceMock<MockType> mock_;
    App app_;
};

// [proxy-async-method-test-start]
TEST_F(ProxyAbstractAppTestFixture, AsyncMethodForwardsToMock)
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
    bool const initResult = app_.init();
    app_.runAsyncMethod(input);

    // ASSERT
    EXPECT_TRUE(initResult);
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
TEST_F(ProxyAbstractAppTestFixture, FireAndForgetMethodForwardsToMock)
{
    // ARRANGE
    App::FireAndForgetPayload payload{{1U, 2U}, {3U, 4U}};
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
    bool const initResult           = app_.init();
    App::App::MwResult const result = app_.runFireAndForgetMethod(payload);

    // ASSERT
    EXPECT_TRUE(initResult);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), kExpectedRequestId);
}

// [proxy-fire-and-forget-test-end]

// [proxy-attribute-get-test-start]
TEST_F(ProxyAbstractAppTestFixture, AttributeGetterForwardsToMock)
{
    // ARRANGE
    App::AttributeGetterCallback getterCallback;
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
    bool const initResult                 = app_.init();
    App::App::MwResult const getterResult = app_.runAttributeGet();

    // ASSERT
    EXPECT_TRUE(initResult);
    ASSERT_TRUE(getterResult.has_value());
    EXPECT_EQ(getterResult.value(), kExpectedGetterRequestId);

    // ARRANGE
    App::AttributeGetterResult const attributeResult{
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
TEST_F(ProxyAbstractAppTestFixture, AttributeSetterForwardsToMock)
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
    bool const initResult                 = app_.init();
    App::App::MwResult const setterResult = app_.runAttributeSet(42U);

    // ASSERT
    EXPECT_TRUE(initResult);
    ASSERT_TRUE(setterResult.has_value());
    EXPECT_EQ(setterResult.value(), kExpectedSetterRequestId);
}

// [proxy-attribute-set-test-end]

// [proxy-attribute-receive-test-start]
TEST_F(ProxyAbstractAppTestFixture, AttributeReceiveForwardsToMock)
{
    // ARRANGE
    App::AttributeReceiveCallback attributeCallback;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleField, setReceiveHandler(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&attributeCallback));

    // ACT
    bool const initResult = app_.init();
    app_.setAttributeReceiveHandler();

    // ASSERT
    EXPECT_TRUE(initResult);

    // ARRANGE
    uint32_t const attributeValue = 42U;

    // ACT
    attributeCallback(attributeValue);

    // ASSERT
    EXPECT_EQ(app_.receivedAttributeValue(), attributeValue);
}

// [proxy-attribute-receive-test-end]

// [proxy-event-receive-test-start]
TEST_F(ProxyAbstractAppTestFixture, EventReceiveForwardsToMock)
{
    // ARRANGE
    App::EventReceiveCallback eventCallback;
    EXPECT_CALL(
        mock_,
        init(
            features::communication::DummyService::internal::InstanceId_1,
            middleware::core::ClusterId::Core1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleBroadcast, setReceiveHandler(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&eventCallback));

    // ACT
    bool const initResult = app_.init();
    app_.setEventReceiveHandler();

    // ASSERT
    EXPECT_TRUE(initResult);

    // ARRANGE
    uint32_t const eventValue = 99U;

    // ACT
    eventCallback(eventValue);

    // ASSERT
    EXPECT_EQ(app_.receivedEventValue(), eventValue);
}

// [proxy-event-receive-test-end]

} // namespace Test
} // namespace App
