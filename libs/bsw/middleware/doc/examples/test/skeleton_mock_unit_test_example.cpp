/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <gtest/gtest.h>

#include "features/communication/dummy_serviceSkeleton.h"
#include "features/communication/dummy_serviceSkeletonMock.h"
#include "gmock/gmock.h"
#include "logger/mock/LoggerMock.h"
#include "memory/mock/AllocatorMock.h"
#include "middleware/core/ResponseBufferBase.h"
#include "middleware/core/types.h"

namespace App
{

class SkeletonApp : public features::communication::DummyService::skeleton::DummyServiceSkeleton
{
public:
    using Base = features::communication::DummyService::skeleton::DummyServiceSkeleton;
    using Foo  = features::communication::DummyService::Foo;
    using FireAndForgetPayload = features::communication::DummyService::FireAndForgetPayload;
    using SkeletonResponseInfo = middleware::core::ResponseBufferBase::SkeletonResponseInfo;

    bool init()
    {
        return Base::init(features::communication::DummyService::internal::InstanceId_1)
               == middleware::core::HRESULT::Ok;
    }

    // [skeleton-app-async-method-start]
    void asyncMethod(Foo const& input, SkeletonResponseInfo& response) override
    {
        Base::asyncMethod(input, response);
    }

    // [skeleton-app-async-method-end]

    // [skeleton-app-fire-and-forget-start]
    void fireAndForgetMethod(
        features::communication::DummyService::FireAndForgetPayload const& payload) override
    {
        Base::fireAndForgetMethod(payload);
    }

    // [skeleton-app-fire-and-forget-end]

    // [skeleton-app-attribute-get-start]
    void get_SimpleFieldAttribute(SkeletonResponseInfo& response) override
    {
        Base::get_SimpleFieldAttribute(response);
    }

    // [skeleton-app-attribute-get-end]

    // [skeleton-app-attribute-set-start]
    void set_SimpleFieldAttribute(uint32_t const& value) override
    {
        Base::set_SimpleFieldAttribute(value);
    }

    // [skeleton-app-attribute-set-end]
};

namespace Test
{

class SkeletonAbstractAppTestFixture : public ::testing::Test
{
public:
    using MockType = features::communication::DummyService::skeleton::DummyServiceSkeletonMock;
    using SkeletonResponseInfo = middleware::core::ResponseBufferBase::SkeletonResponseInfo;
    using Foo                  = SkeletonApp::Foo;

    SkeletonAbstractAppTestFixture()                                                 = default;
    ~SkeletonAbstractAppTestFixture() override                                       = default;
    SkeletonAbstractAppTestFixture(SkeletonAbstractAppTestFixture const&)            = delete;
    SkeletonAbstractAppTestFixture(SkeletonAbstractAppTestFixture&&)                 = delete;
    SkeletonAbstractAppTestFixture& operator=(SkeletonAbstractAppTestFixture const&) = delete;
    SkeletonAbstractAppTestFixture& operator=(SkeletonAbstractAppTestFixture&&)      = delete;

public:
    void SetUp() final
    {
        middleware::memory::test::AllocatorMock::setAllocatorMock(allocatorMock_);
    }

protected:
    testing::NiceMock<middleware::logger::test::mock::LoggerMock> loggerMock_;
    testing::NiceMock<middleware::memory::test::AllocatorMock> allocatorMock_;
    testing::NiceMock<MockType> mock_;
    SkeletonApp app_;
};

// [skeleton-async-method-test-start]
TEST_F(SkeletonAbstractAppTestFixture, SkeletonAsyncMethodForwardsToMock)
{
    // ARRANGE
    SkeletonResponseInfo response{};
    Foo input{1U, 2U};
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, asyncMethod(::testing::Ref(input), ::testing::Ref(response)));

    // ACT
    bool const initResult = app_.init();
    app_.asyncMethod(input, response);

    // ASSERT
    EXPECT_TRUE(initResult);
}

// [skeleton-async-method-test-end]

// [skeleton-fire-and-forget-test-start]
TEST_F(SkeletonAbstractAppTestFixture, SkeletonFireAndForgetMethodForwardsToMock)
{
    // ARRANGE
    features::communication::DummyService::FireAndForgetPayload payload{{1U, 2U}, {3U, 4U}};
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, fireAndForgetMethod(::testing::Ref(payload)));

    // ACT
    bool const initResult = app_.init();
    app_.fireAndForgetMethod(payload);

    // ASSERT
    EXPECT_TRUE(initResult);
}

// [skeleton-fire-and-forget-test-end]

// [skeleton-attribute-get-test-start]
TEST_F(SkeletonAbstractAppTestFixture, SkeletonAttributeGetterForwardsToMock)
{
    // ARRANGE
    SkeletonResponseInfo response{};
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, get_SimpleFieldAttribute(::testing::Ref(response)));

    // ACT
    bool const initResult = app_.init();
    app_.get_SimpleFieldAttribute(response);

    // ASSERT
    EXPECT_TRUE(initResult);
}

// [skeleton-attribute-get-test-end]

// [skeleton-attribute-set-test-start]
TEST_F(SkeletonAbstractAppTestFixture, SkeletonAttributeSetterForwardsToMock)
{
    // ARRANGE
    uint32_t const value = 42U;
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, set_SimpleFieldAttribute(::testing::Ref(value)));

    // ACT
    bool const initResult = app_.init();
    app_.set_SimpleFieldAttribute(value);

    // ASSERT
    EXPECT_TRUE(initResult);
}

// [skeleton-attribute-set-test-end]

// [skeleton-attribute-send-test-start]
TEST_F(SkeletonAbstractAppTestFixture, SkeletonAttributeEventForwardsToMock)
{
    // ARRANGE
    uint32_t const attributeValue = 42U;
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleField, send(attributeValue))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));

    // ACT
    bool const initResult                           = app_.init();
    middleware::core::HRESULT const attributeResult = app_.simpleField.send(attributeValue);

    // ASSERT
    EXPECT_TRUE(initResult);
    EXPECT_EQ(attributeResult, middleware::core::HRESULT::Ok);
}

// [skeleton-attribute-send-test-end]

// [skeleton-event-send-test-start]
TEST_F(SkeletonAbstractAppTestFixture, SkeletonEventForwardsToMock)
{
    // ARRANGE
    uint32_t const broadcastValue = 99U;
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_.simpleBroadcast, send(broadcastValue))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));

    // ACT
    bool const initResult                           = app_.init();
    middleware::core::HRESULT const broadcastResult = app_.simpleBroadcast.send(broadcastValue);

    // ASSERT
    EXPECT_TRUE(initResult);
    EXPECT_EQ(broadcastResult, middleware::core::HRESULT::Ok);
}

// [skeleton-event-send-test-end]

} // namespace Test
} // namespace App
