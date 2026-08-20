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
#include "skeleton_app.h"

namespace application
{
namespace test
{

class SkeletonAppTestFixture : public ::testing::Test
{
public:
    using MockType = features::communication::DummyService::skeleton::DummyServiceSkeletonMock;
    using SkeletonResponseInfo = middleware::core::ResponseBufferBase::SkeletonResponseInfo;
    using Foo                  = SkeletonApp::Foo;

    SkeletonAppTestFixture()                                         = default;
    ~SkeletonAppTestFixture() override                               = default;
    SkeletonAppTestFixture(SkeletonAppTestFixture const&)            = delete;
    SkeletonAppTestFixture(SkeletonAppTestFixture&&)                 = delete;
    SkeletonAppTestFixture& operator=(SkeletonAppTestFixture const&) = delete;
    SkeletonAppTestFixture& operator=(SkeletonAppTestFixture&&)      = delete;

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
TEST_F(SkeletonAppTestFixture, SkeletonAsyncMethodForwardsToMock)
{
    // ARRANGE
    SkeletonResponseInfo response{};
    Foo input{1U, 2U};
    EXPECT_CALL(mock_, init(features::communication::DummyService::internal::InstanceId_1))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));
    EXPECT_CALL(mock_, asyncMethod(::testing::Ref(input), ::testing::Ref(response)));
    EXPECT_CALL(
        mock_,
        respond(
            ::testing::Ref(response),
            ::testing::Matcher<SkeletonApp::Baz const&>(::testing::Truly(
                [](SkeletonApp::Baz const& result)
                { return result.a == 9U && result.b == 8U && result.c == 7U && result.d == 6U; })),
            true))
        .WillOnce(testing::Return(middleware::core::HRESULT::Ok));

    // ACT
    bool const initResult = app_.init();
    app_.asyncMethod(input, response);
    app_.execute();

    // ASSERT
    EXPECT_TRUE(initResult);
}

// [skeleton-async-method-test-end]

// [skeleton-fire-and-forget-test-start]
TEST_F(SkeletonAppTestFixture, SkeletonFireAndForgetMethodForwardsToMock)
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
TEST_F(SkeletonAppTestFixture, SkeletonAttributeGetterForwardsToMock)
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
TEST_F(SkeletonAppTestFixture, SkeletonAttributeSetterForwardsToMock)
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
TEST_F(SkeletonAppTestFixture, SkeletonAttributeEventForwardsToMock)
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
TEST_F(SkeletonAppTestFixture, SkeletonEventForwardsToMock)
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

} // namespace test
} // namespace application
