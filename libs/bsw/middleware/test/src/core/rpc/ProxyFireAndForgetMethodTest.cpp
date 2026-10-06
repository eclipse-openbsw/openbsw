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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/mock/ProxyMock.h"
#include "memory/mock/AllocatorMock.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/rpc/ProxyFireAndForgetMethod.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace middleware::rpc::test
{

/// A payload type larger than Message::MAX_PAYLOAD_SIZE to force external allocation.
struct LargePayload
{
    etl::array<uint8_t, core::Message::MAX_PAYLOAD_SIZE + 16U> data{};
};

template<typename DataType>
struct TestConfig
{
    using InputType = DataType;
    static constexpr bool USES_EXTERNAL_ALLOCATION
        = sizeof(DataType) > core::Message::MAX_PAYLOAD_SIZE;
};

template<typename Config>
class ProxyFireAndForgetTestBase : public ::testing::Test
{
public:
    void SetUp() override
    {
        ON_CALL(_proxyMock, isInitialized()).WillByDefault(Return(true));
        ON_CALL(_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::Ok));
        memory::test::AllocatorMock::setAllocatorMock(_allocatorMock);
    }

protected:
    static constexpr uint16_t FIRE_FORGET_METHOD_ID = 7U;

    NiceMock<core::test::ProxyMock> _proxyMock{};
    NiceMock<memory::test::AllocatorMock> _allocatorMock{};
    ProxyFireAndForgetMethod _method{};
};

// ---------------------------------------------------------------------------
// ProxyFireAndForgetMethodTestSuite — runs for all configs
// ---------------------------------------------------------------------------

template<typename Config>
class ProxyFireAndForgetMethodTestSuite : public ProxyFireAndForgetTestBase<Config>
{};

using AllConfigs = ::testing::Types<TestConfig<int>, TestConfig<LargePayload>>;
TYPED_TEST_SUITE(ProxyFireAndForgetMethodTestSuite, AllConfigs);

TYPED_TEST(ProxyFireAndForgetMethodTestSuite, CallMethodSucceeds)
{
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
    EXPECT_TRUE(result.has_value());
}

TYPED_TEST(ProxyFireAndForgetMethodTestSuite, CallMethodFailsWhenProxyNotInitialized)
{
    ON_CALL(this->_proxyMock, isInitialized()).WillByDefault(Return(false));
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::NotRegistered);
}

TYPED_TEST(ProxyFireAndForgetMethodTestSuite, CallMethodFailsWhenSendFails)
{
    ON_CALL(this->_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::QueueFull));
    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::QueueFull);
}

TYPED_TEST(ProxyFireAndForgetMethodTestSuite, CallMethodCanBeCalledMultipleTimes)
{
    for (int i = 0; i < 10; ++i)
    {
        auto const result = this->_method.callMethod(
            this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
        EXPECT_TRUE(result.has_value());
    }
}

// ---------------------------------------------------------------------------
// ProxyFireAndForgetVoidInputSuite — tests for void InputType
// ---------------------------------------------------------------------------

class ProxyFireAndForgetVoidInputSuite : public ::testing::Test
{
public:
    void SetUp() override
    {
        ON_CALL(_proxyMock, isInitialized()).WillByDefault(Return(true));
        ON_CALL(_proxyMock, sendMessage(_)).WillByDefault(Return(core::HRESULT::Ok));
    }

protected:
    static constexpr uint16_t FIRE_FORGET_METHOD_ID = 7U;

    NiceMock<core::test::ProxyMock> _proxyMock{};
    ProxyFireAndForgetMethod _method{};
};

TEST_F(ProxyFireAndForgetVoidInputSuite, CallMethodSucceeds)
{
    auto const result = _method.callMethod(_proxyMock, FIRE_FORGET_METHOD_ID);
    EXPECT_TRUE(result.has_value());
}

TEST_F(ProxyFireAndForgetVoidInputSuite, CallMethodFailsWhenProxyNotInitialized)
{
    ON_CALL(_proxyMock, isInitialized()).WillByDefault(Return(false));
    auto const result = _method.callMethod(_proxyMock, FIRE_FORGET_METHOD_ID);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::NotRegistered);
}

// ---------------------------------------------------------------------------
// ProxyFireAndForgetExternalAllocSuite — runs only for large payload configs
// ---------------------------------------------------------------------------

template<typename Config>
class ProxyFireAndForgetExternalAllocSuite : public ProxyFireAndForgetTestBase<Config>
{};

using ExternalAllocConfigs = ::testing::Types<TestConfig<LargePayload>>;
TYPED_TEST_SUITE(ProxyFireAndForgetExternalAllocSuite, ExternalAllocConfigs);

// --- External allocation tests ---

TYPED_TEST(ProxyFireAndForgetExternalAllocSuite, CallMethodFailsWhenAllocatorReturnsNull)
{
    ON_CALL(this->_allocatorMock, allocateImpl).WillByDefault(Return(nullptr));

    auto const result = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), core::HRESULT::CannotAllocatePayload);
}

TYPED_TEST(ProxyFireAndForgetExternalAllocSuite, AllocationFailureDoesNotPreventSubsequentCalls)
{
    // First call: allocation fails
    ON_CALL(this->_allocatorMock, allocateImpl).WillByDefault(Return(nullptr));
    auto const failedResult = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
    ASSERT_FALSE(failedResult.has_value());
    EXPECT_EQ(failedResult.error(), core::HRESULT::CannotAllocatePayload);

    // Restore allocator defaults
    memory::test::AllocatorMock::setAllocatorMock(this->_allocatorMock);

    // Second call should succeed
    auto const successResult = this->_method.callMethod(
        this->_proxyMock, typename TypeParam::InputType{}, this->FIRE_FORGET_METHOD_ID);
    ASSERT_TRUE(successResult.has_value());
}

} // namespace middleware::rpc::test
