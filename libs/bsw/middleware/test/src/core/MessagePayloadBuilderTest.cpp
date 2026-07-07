/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <cstdint>

#include <etl/memory.h>
#include <etl/type_traits.h>
#include <etl/vector.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger/DslLogger.h"
#include "memory/mock/AllocatorMock.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"

namespace middleware::core::test
{
template<typename T>
struct IntegralWrapper
{
    static_assert(::etl::is_integral_v<T>, "IntegralWrapper requires an integral type");
    T value;

    bool operator==(IntegralWrapper const& rhs) const { return value == rhs.value; }

    static IntegralWrapper make() { return {static_cast<T>(~T{0})}; }
};

template<typename T, size_t SIZE>
struct ArrayWrapper
{
    ::etl::array<T, SIZE> buffer;

    bool operator==(ArrayWrapper const& rhs) const { return buffer == rhs.buffer; }

    static ArrayWrapper make()
    {
        ArrayWrapper obj{};
        obj.buffer.fill(static_cast<T>(~T{0}));
        return obj;
    }
};

template<typename T, size_t SIZE>
struct VectorWrapper
{
    ::etl::vector<T, SIZE> buffer;

    bool operator==(VectorWrapper const& rhs) const { return buffer == rhs.buffer; }

    static VectorWrapper make()
    {
        VectorWrapper obj{};
        for (size_t i = 0U; i < SIZE; ++i)
        {
            obj.buffer.push_back(static_cast<T>(~T{0}));
        }
        return obj;
    }
};

using SmallTypes = ::testing::Types<
    IntegralWrapper<uint8_t>,
    IntegralWrapper<uint16_t>,
    IntegralWrapper<uint32_t>,
    IntegralWrapper<uint64_t>,
    ArrayWrapper<uint8_t, 1U>,
    ArrayWrapper<uint16_t, 1U>,
    ArrayWrapper<uint32_t, 1U>,
    ArrayWrapper<uint64_t, 1U>,
    VectorWrapper<uint8_t, 1U>,
    VectorWrapper<uint16_t, 1U>,
    VectorWrapper<uint32_t, 1U>,
    VectorWrapper<uint64_t, 1U>>;

using BigTypes = ::testing::Types<
    ArrayWrapper<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    ArrayWrapper<uint16_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    ArrayWrapper<uint32_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    ArrayWrapper<uint64_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    VectorWrapper<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    VectorWrapper<uint16_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    VectorWrapper<uint32_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    VectorWrapper<uint64_t, Message::MAX_PAYLOAD_SIZE + 1U>>;

using SmallBuffers = ::testing::Types<
    ::etl::array<uint8_t, 1U>,
    ::etl::array<uint8_t, 2U>,
    ::etl::array<uint8_t, 4U>,
    ::etl::array<uint8_t, 8U>,
    ::etl::array<uint8_t, 16U>,
    ::etl::array<uint8_t, 32U>,
    ::etl::array<uint8_t, Message::MAX_PAYLOAD_SIZE - 1U>,
    ::etl::array<uint8_t, Message::MAX_PAYLOAD_SIZE>>;

using BigBuffers = ::testing::Types<
    ::etl::array<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U>,
    ::etl::array<uint8_t, 64U>,
    ::etl::array<uint8_t, 128U>,
    ::etl::array<uint8_t, 256U>,
    ::etl::array<uint8_t, 511U>>;

class AllocatorImpl
{
public:
    AllocatorImpl()                                      = default;
    ~AllocatorImpl()                                     = default;
    AllocatorImpl(AllocatorImpl const& other)            = delete;
    AllocatorImpl(AllocatorImpl&& other)                 = delete;
    AllocatorImpl& operator=(AllocatorImpl const& other) = delete;
    AllocatorImpl& operator=(AllocatorImpl&& other)      = delete;

    template<size_t MAX_COUNT, size_t MAX_SIZE>
    using Storage = ::etl::vector<::etl::array<uint8_t, MAX_SIZE>, MAX_COUNT>;

    uint8_t* allocate(uint32_t const size)
    {
        uint8_t* res{nullptr};
        if ((size <= decltype(_pool128)::value_type::SIZE) && !_pool128.full())
        {
            res = _pool128.emplace_back().data();
        }
        else if ((size <= decltype(_pool256)::value_type::SIZE) && !_pool256.full())
        {
            res = _pool256.emplace_back().data();
        }
        else if ((size <= decltype(_pool512)::value_type::SIZE) && !_pool512.full())
        {
            res = _pool512.emplace_back().data();
        }

        return res;
    }

    void deallocate(void* ptr)
    {
        if (!_pool128.empty())
        {
            if (reinterpret_cast<decltype(_pool128)::iterator>(ptr) == _pool128.data())
            {
                _pool128.at(0).fill(0U);
                _pool128.clear();
                return;
            }
        }
        if (!_pool256.empty())
        {
            if (reinterpret_cast<decltype(_pool256)::iterator>(ptr) == _pool256.data())
            {
                _pool256.at(0).fill(0U);
                _pool256.clear();
                return;
            }
        }
        if (!_pool512.empty())
        {
            if (reinterpret_cast<decltype(_pool512)::iterator>(ptr) == _pool512.data())
            {
                _pool512.at(0).fill(0U);
                _pool512.clear();
                return;
            }
        }
        FAIL() << "Pointer does not belong to any pool.";
    }

    bool isAnyPoolFull() const { return _pool128.full() || _pool256.full() || _pool512.full(); }

    uint8_t* regionStart() { return reinterpret_cast<uint8_t*>(_pool128.data()); }

    bool isPtrValid(void const* const ptr)
    {
        return (ptr == _pool128.data()) || (ptr == _pool256.data()) || (ptr == _pool512.data());
    }

private:
    Storage<1U, 128U> _pool128;
    Storage<1U, 256U> _pool256;
    Storage<1U, 512U> _pool512;
};

class TestMessagePayloadBuilder : public ::testing::Test
{
public:
    void SetUp() final
    {
        _loggerMock.setup();
        memory::test::AllocatorMock::setAllocatorMock(_allocatorMock);
        ON_CALL(_allocatorMock, allocateImpl)
            .WillByDefault(
                [this](uint32_t const size) -> uint8_t* { return _allocator.allocate(size); });
        ON_CALL(_allocatorMock, deallocateImpl)
            .WillByDefault([this](void* ptr) -> void { _allocator.deallocate(ptr); });
        ON_CALL(_allocatorMock, regionStartImpl)
            .WillByDefault([this](void) -> uint8_t* { return _allocator.regionStart(); });
        ON_CALL(_allocatorMock, isPtrValidImpl)
            .WillByDefault(
                [this](void const* const ptr) -> bool { return _allocator.isPtrValid(ptr); });
    }

    void TearDown() final { _loggerMock.teardown(); }

    AllocatorImpl const& getAllocatorImpl() { return _allocator; }

protected:
    middleware::logger::test::DslLogger _loggerMock{};
    testing::NiceMock<memory::test::AllocatorMock> _allocatorMock{};
    AllocatorImpl _allocator{};
};

namespace
{
/// Helper to create a default/empty message for allocator tests.
/// Message has no public default constructor, so we use the factory with dummy values.
Message makeEmptyMessage() { return Message::createRequest(0U, 0U, 0U, 0U, 0U, 0U, 0U); }
} // namespace

template<typename T>
class TestMessagePayloadBuilderSmallUserDefinedType : public TestMessagePayloadBuilder
{};

TYPED_TEST_SUITE(TestMessagePayloadBuilderSmallUserDefinedType, SmallTypes);

TYPED_TEST(TestMessagePayloadBuilderSmallUserDefinedType, AllocateInternal)
{
    static_assert(
        sizeof(TypeParam) <= Message::MAX_PAYLOAD_SIZE,
        "TypeParam must fit in internal payload, this is a test logic error");

    // ARRANGE
    auto const obj = TypeParam::make();
    Message msg    = makeEmptyMessage();

    // ACT
    core::HRESULT ret     = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const& storedObj = MessagePayloadBuilder::getInstance().readPayload<TypeParam>(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_FALSE(msg.hasExternalPayload());
    EXPECT_EQ(obj, storedObj);

    // ACT
    MessagePayloadBuilder::deallocate(msg);
}

template<typename T>
class TestMessagePayloadBuilderBigUserDefinedType : public TestMessagePayloadBuilder
{};

TYPED_TEST_SUITE(TestMessagePayloadBuilderBigUserDefinedType, BigTypes);

TYPED_TEST(TestMessagePayloadBuilderBigUserDefinedType, AllocateExternal)
{
    static_assert(
        sizeof(TypeParam) > Message::MAX_PAYLOAD_SIZE,
        "TypeParam must NOT fit in internal payload, this is a test logic error");

    // ARRANGE
    auto const obj = TypeParam::make();
    Message msg    = makeEmptyMessage();

    // ACT
    core::HRESULT ret     = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const& storedObj = MessagePayloadBuilder::getInstance().readPayload<TypeParam>(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_TRUE(msg.hasExternalPayload());
    EXPECT_EQ(obj, storedObj);

    // ACT
    MessagePayloadBuilder::deallocate(msg);
}

TYPED_TEST(TestMessagePayloadBuilderBigUserDefinedType, AllocateExternalShared)
{
    static_assert(
        sizeof(TypeParam) > Message::MAX_PAYLOAD_SIZE,
        "TypeParam must NOT fit in internal payload, this is a test logic error");

    // ARRANGE
    auto const obj                   = TypeParam::make();
    Message msg                      = Message::createEvent(123U, 128U, 1U, 0U);
    uint8_t const numberOfReferences = 5U;

    // ACT
    core::HRESULT ret = MessagePayloadBuilder::getInstance().allocate(obj, msg, numberOfReferences);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_TRUE(msg.hasExternalPayload());

    for (size_t readings = 0U; readings < numberOfReferences; ++readings)
    {
        EXPECT_TRUE(this->getAllocatorImpl().isAnyPoolFull());
        auto const& storedObj = MessagePayloadBuilder::getInstance().readPayload<TypeParam>(msg);
        EXPECT_EQ(obj, storedObj);
        MessagePayloadBuilder::deallocate(msg);
    }
    EXPECT_FALSE(this->getAllocatorImpl().isAnyPoolFull());
}

template<typename T>
class TestMessagePayloadBuilderSmallSpan : public TestMessagePayloadBuilder
{};

TYPED_TEST_SUITE(TestMessagePayloadBuilderSmallSpan, SmallBuffers);

TYPED_TEST(TestMessagePayloadBuilderSmallSpan, AllocateInternal)
{
    using value_type = typename TypeParam::value_type;
    static_assert(
        sizeof(TypeParam) <= Message::MAX_PAYLOAD_SIZE,
        "TypeParam must fit in internal payload, this is a test logic error");

    // ARRANGE
    TypeParam obj;
    obj.fill(static_cast<value_type>(~value_type{0U}));

    ::etl::span<uint8_t const> const bytes{obj};
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret = MessagePayloadBuilder::getInstance().allocate(bytes, msg);
    ::etl::span<uint8_t const> const storedBytes
        = MessagePayloadBuilder::getInstance().readRawPayload(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_FALSE(msg.hasExternalPayload());
    EXPECT_TRUE(::etl::equal(bytes.begin(), bytes.end(), storedBytes.begin(), storedBytes.end()));

    // ACT
    MessagePayloadBuilder::deallocate(msg);
}

template<typename T>
class TestMessagePayloadBuilderBigSpan : public TestMessagePayloadBuilder
{};

TYPED_TEST_SUITE(TestMessagePayloadBuilderBigSpan, BigBuffers);

TYPED_TEST(TestMessagePayloadBuilderBigSpan, AllocateExternal)
{
    using value_type = typename TypeParam::value_type;
    static_assert(
        sizeof(TypeParam) > Message::MAX_PAYLOAD_SIZE,
        "TypeParam must NOT fit in internal payload, this is a test logic error");

    // ARRANGE
    TypeParam obj;
    obj.fill(static_cast<value_type>(~value_type{0U}));

    ::etl::span<uint8_t const> const bytes{obj};
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret = MessagePayloadBuilder::getInstance().allocate(bytes, msg);
    ::etl::span<uint8_t const> const storedBytes
        = MessagePayloadBuilder::getInstance().readRawPayload(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_TRUE(msg.hasExternalPayload());
    EXPECT_TRUE(::etl::equal(bytes.begin(), bytes.end(), storedBytes.begin(), storedBytes.end()));

    // ACT
    MessagePayloadBuilder::deallocate(msg);
}

TYPED_TEST(TestMessagePayloadBuilderBigSpan, AllocateExternalShared)
{
    using value_type = typename TypeParam::value_type;
    static_assert(
        sizeof(TypeParam) > Message::MAX_PAYLOAD_SIZE,
        "TypeParam must NOT fit in internal payload, this is a test logic error");

    // ARRANGE
    TypeParam obj;
    obj.fill(static_cast<value_type>(~value_type{0U}));

    ::etl::span<uint8_t const> const bytes{obj};
    Message msg                      = Message::createEvent(123U, 128U, 1U, 0U);
    uint8_t const numberOfReferences = 5U;

    // ACT
    core::HRESULT ret
        = MessagePayloadBuilder::getInstance().allocate(bytes, msg, numberOfReferences);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_TRUE(msg.hasExternalPayload());

    for (size_t readings = 0U; readings < numberOfReferences; ++readings)
    {
        EXPECT_TRUE(this->getAllocatorImpl().isAnyPoolFull());
        ::etl::span<uint8_t const> const storedBytes
            = MessagePayloadBuilder::getInstance().readRawPayload(msg);
        EXPECT_TRUE(
            ::etl::equal(bytes.begin(), bytes.end(), storedBytes.begin(), storedBytes.end()));
        MessagePayloadBuilder::deallocate(msg);
    }
    EXPECT_FALSE(this->getAllocatorImpl().isAnyPoolFull());
}

TEST_F(TestMessagePayloadBuilder, AllocateExternalFailsForObject)
{
    // ARRANGE
    auto const obj = ArrayWrapper<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U>::make();
    Message msg1   = makeEmptyMessage();
    Message msg2   = makeEmptyMessage();
    Message msg3   = makeEmptyMessage();
    Message msg4   = makeEmptyMessage();

    // ACT
    core::HRESULT firstAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg1);
    core::HRESULT secondAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg2);
    core::HRESULT thirdAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg3);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::Allocation,
        HRESULT::CannotAllocatePayload,
        msg4.getHeader().srcClusterId,
        msg4.getHeader().tgtClusterId,
        msg4.getHeader().serviceId,
        msg4.getHeader().serviceInstanceId,
        msg4.getHeader().memberId,
        msg4.getHeader().requestId,
        static_cast<uint32_t>(sizeof(obj)));
    core::HRESULT fourthAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg4);

    // ASSERT
    EXPECT_EQ(firstAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(secondAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(thirdAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(fourthAllocationResult, core::HRESULT::CannotAllocatePayload);
    MessagePayloadBuilder::deallocate(msg1);
    MessagePayloadBuilder::deallocate(msg2);
    MessagePayloadBuilder::deallocate(msg3);
}

TEST_F(TestMessagePayloadBuilder, AllocateExternalSharedFailsForObject)
{
    // ARRANGE
    auto const obj = ArrayWrapper<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U>::make();
    Message msg1   = makeEmptyMessage();
    Message msg2   = makeEmptyMessage();
    Message msg3   = makeEmptyMessage();
    Message msg4   = Message::createEvent(123U, 128U, 1U, 0U);

    // ACT
    core::HRESULT firstAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg1);
    core::HRESULT secondAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg2);
    core::HRESULT thirdAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg3);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::Allocation,
        HRESULT::CannotAllocatePayload,
        msg4.getHeader().srcClusterId,
        msg4.getHeader().tgtClusterId,
        msg4.getHeader().serviceId,
        msg4.getHeader().serviceInstanceId,
        msg4.getHeader().memberId,
        msg4.getHeader().requestId,
        static_cast<uint32_t>(sizeof(obj)));
    core::HRESULT fourthAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg4);

    // ASSERT
    EXPECT_EQ(firstAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(secondAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(thirdAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(fourthAllocationResult, core::HRESULT::CannotAllocatePayload);
    MessagePayloadBuilder::deallocate(msg1);
    MessagePayloadBuilder::deallocate(msg2);
    MessagePayloadBuilder::deallocate(msg3);
}

TEST_F(TestMessagePayloadBuilder, AllocateExternalFailsForSpan)
{
    // ARRANGE
    ::etl::array<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U> buffer{};
    buffer.fill(0xFFU);
    ::etl::span<uint8_t const> const obj{buffer};
    Message msg1 = makeEmptyMessage();
    Message msg2 = makeEmptyMessage();
    Message msg3 = makeEmptyMessage();
    Message msg4 = makeEmptyMessage();

    // ACT
    core::HRESULT firstAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg1);
    core::HRESULT secondAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg2);
    core::HRESULT thirdAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg3);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::Allocation,
        HRESULT::CannotAllocatePayload,
        msg4.getHeader().srcClusterId,
        msg4.getHeader().tgtClusterId,
        msg4.getHeader().serviceId,
        msg4.getHeader().serviceInstanceId,
        msg4.getHeader().memberId,
        msg4.getHeader().requestId,
        static_cast<uint32_t>(obj.size_bytes()));
    core::HRESULT fourthAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg4);

    // ASSERT
    EXPECT_EQ(firstAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(secondAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(thirdAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(fourthAllocationResult, core::HRESULT::CannotAllocatePayload);
    MessagePayloadBuilder::deallocate(msg1);
    MessagePayloadBuilder::deallocate(msg2);
    MessagePayloadBuilder::deallocate(msg3);
}

TEST_F(TestMessagePayloadBuilder, AllocateExternalSharedFailsForSpan)
{
    // ARRANGE
    ::etl::array<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U> buffer{};
    buffer.fill(0xFFU);
    ::etl::span<uint8_t const> const obj{buffer};
    Message msg1 = makeEmptyMessage();
    Message msg2 = makeEmptyMessage();
    Message msg3 = makeEmptyMessage();
    Message msg4 = Message::createEvent(123U, 128U, 1U, 0U);

    // ACT
    core::HRESULT firstAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg1);
    core::HRESULT secondAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg2);
    core::HRESULT thirdAllocationResult  = MessagePayloadBuilder::getInstance().allocate(obj, msg3);

    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::Allocation,
        HRESULT::CannotAllocatePayload,
        msg4.getHeader().srcClusterId,
        msg4.getHeader().tgtClusterId,
        msg4.getHeader().serviceId,
        msg4.getHeader().serviceInstanceId,
        msg4.getHeader().memberId,
        msg4.getHeader().requestId,
        static_cast<uint32_t>(obj.size_bytes()));
    core::HRESULT fourthAllocationResult = MessagePayloadBuilder::getInstance().allocate(obj, msg4);

    // ASSERT
    EXPECT_EQ(firstAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(secondAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(thirdAllocationResult, core::HRESULT::Ok);
    EXPECT_EQ(fourthAllocationResult, core::HRESULT::CannotAllocatePayload);
    MessagePayloadBuilder::deallocate(msg1);
    MessagePayloadBuilder::deallocate(msg2);
    MessagePayloadBuilder::deallocate(msg3);
}

} // namespace middleware::core::test
