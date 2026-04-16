// Copyright 2025 BMW AG

#include <cstdint>

#include <etl/byte_stream.h>
#include <etl/memory.h>
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

struct SmallTrivialType
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
};

struct BigTrivialType
{
    uint64_t a;
    uint64_t b;
    uint64_t c;
    uint64_t d;
    uint64_t e;
    uint64_t f;
    uint64_t g;
    uint64_t h;
    uint64_t i;
};

} // namespace middleware::core::test

/// ETL user-defined type-traits specializations for trivially copyable test types.
/// Required because OpenBSW ETL is configured with ETL_USER_DEFINED_TYPE_TRAITS,
/// which only recognises arithmetic and pointer types as trivially copyable by default.
namespace etl
{
template<>
struct is_trivially_copyable<middleware::core::test::SmallTrivialType> : public etl::true_type
{};

template<>
struct is_trivially_copyable<middleware::core::test::BigTrivialType> : public etl::true_type
{};

template<size_t N>
struct is_trivially_copyable<etl::array<uint8_t, N>> : public etl::true_type
{};

template<size_t N>
struct is_trivially_copyable<etl::array<uint8_t, N> const> : public etl::true_type
{};
} // namespace etl

namespace middleware::core::test
{

namespace
{
/// Helper to create a default/empty message for allocator tests.
/// Message has no public default constructor, so we use the factory with dummy values.
Message makeEmptyMessage() { return Message::createRequest(0U, 0U, 0U, 0U, 0U, 0U, 0U); }
} // namespace

struct SmallNonTrivialType
{
    etl::vector<uint8_t, sizeof(SmallTrivialType)> data{};

    struct AllocationPolicy
    {
        static void serialize(SmallNonTrivialType const& obj, etl::span<uint8_t>& buffer)
        {
            etl::byte_stream_writer writer{buffer, etl::endian::native};
            writer.write_unchecked(static_cast<uint32_t>(obj.data.size()));
            for (auto element : obj.data)
            {
                writer.write_unchecked(element);
            }
        }

        static size_t getNeededSize(SmallNonTrivialType const& obj)
        {
            return sizeof(uint32_t) + (obj.data.size() * sizeof(decltype(obj.data)::value_type));
        }

        static SmallNonTrivialType deserialize(etl::span<uint8_t> const& serializedBuffer)
        {
            etl::byte_stream_reader reader{
                serializedBuffer.begin(), serializedBuffer.end(), etl::endian::native};
            SmallNonTrivialType obj{};
            uint32_t const size = reader.read_unchecked<uint32_t>();
            for (uint32_t idx = 0U; idx < size; ++idx)
            {
                obj.data.push_back(reader.read_unchecked<decltype(obj.data)::value_type>());
            }

            return obj;
        }
    };
};

struct BigNonTrivialType
{
    uint32_t a;
    uint32_t b;
    etl::vector<uint8_t, Message::MAX_PAYLOAD_SIZE> data{};

    struct AllocationPolicy
    {
        static void serialize(BigNonTrivialType const& obj, etl::span<uint8_t>& buffer)
        {
            etl::byte_stream_writer writer{buffer, etl::endian::native};
            writer.write_unchecked(static_cast<uint32_t>(obj.a));
            writer.write_unchecked(static_cast<uint32_t>(obj.b));
            writer.write_unchecked(static_cast<uint32_t>(obj.data.size()));
            for (auto element : obj.data)
            {
                writer.write_unchecked(element);
            }
        }

        static size_t getNeededSize(BigNonTrivialType const& obj)
        {
            return sizeof(obj.a) + sizeof(obj.b) + sizeof(uint32_t)
                   + (obj.data.size() * sizeof(decltype(obj.data)::value_type));
        }

        static BigNonTrivialType deserialize(etl::span<uint8_t> const& serializedBuffer)
        {
            etl::byte_stream_reader reader{
                serializedBuffer.begin(), serializedBuffer.end(), etl::endian::native};
            BigNonTrivialType obj{};
            obj.a               = reader.read_unchecked<decltype(obj.a)>();
            obj.b               = reader.read_unchecked<decltype(obj.b)>();
            uint32_t const size = reader.read_unchecked<uint32_t>();
            for (uint32_t idx = 0U; idx < size; ++idx)
            {
                obj.data.push_back(reader.read_unchecked<decltype(obj.data)::value_type>());
            }

            return obj;
        }
    };
};

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
    using Storage = etl::vector<etl::array<uint8_t, MAX_SIZE>, MAX_COUNT>;

    enum class PoolId : uint8_t
    {
        Pool128 = 0U,
        Pool256,
        Pool512
    };

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
            }
        }
        else if (!_pool256.empty())
        {
            if (reinterpret_cast<decltype(_pool256)::iterator>(ptr) == _pool256.data())
            {
                _pool256.at(0).fill(0U);
                _pool256.clear();
            }
        }
        else if (!_pool512.empty())
        {
            if (reinterpret_cast<decltype(_pool512)::iterator>(ptr) == _pool512.data())
            {
                _pool512.at(0).fill(0U);
                _pool512.clear();
            }
        }
        else
        {
            FAIL() << "Pointer does not belong to any pool.";
        }
    }

    bool isAllocatorPoolFull(PoolId const poolId) const
    {
        bool ret = false;

        switch (poolId)
        {
            case PoolId::Pool128: ret = _pool128.full(); break;
            case PoolId::Pool256: ret = _pool256.full(); break;
            case PoolId::Pool512: ret = _pool512.full(); break;
            default:              break;
        }

        return ret;
    }

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

TEST_F(TestMessagePayloadBuilder, TestSmallTrivialType)
{
    // ARRANGE
    SmallTrivialType const obj{
        0xF1359EA0U,
        0x51314BA1U,
        0x1289CEA2U,
        0x902C37FFU,
    };
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret    = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<SmallTrivialType>(msg);
    MessagePayloadBuilder::deallocate(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.c, storedObj.c);
    EXPECT_EQ(obj.d, storedObj.d);
}

TEST_F(TestMessagePayloadBuilder, TestSmallSpan)
{
    // ARRANGE
    etl::array<uint8_t, 8U> const buffer_{0x0DU, 0x0EU, 0x0AU, 0x0DU, 0x0BU, 0x0EU, 0x0EU, 0x0FU};
    etl::span<uint8_t const> const obj{buffer_};
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret    = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<decltype(buffer_)>(msg);

    // ASSERT
    EXPECT_FALSE(msg.hasExternalPayload());
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_TRUE(etl::equal(obj.begin(), obj.end(), storedObj.begin(), storedObj.end()));

    // ACT
    MessagePayloadBuilder::deallocate(msg);
}

TEST_F(TestMessagePayloadBuilder, TestBigSpan)
{
    // ARRANGE
    etl::array<uint8_t, Message::MAX_PAYLOAD_SIZE + 1U> buffer_{};
    etl::fill(buffer_.begin(), buffer_.end(), 0x00U);
    etl::span<uint8_t const> const obj{buffer_};
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret    = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<decltype(buffer_)>(msg);

    // ASSERT
    EXPECT_TRUE(msg.hasExternalPayload());
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_TRUE(etl::equal(obj.begin(), obj.end(), storedObj.begin(), storedObj.end()));

    // ACT
    MessagePayloadBuilder::deallocate(msg);
}

TEST_F(TestMessagePayloadBuilder, TestBigTrivialType)
{
    // ARRANGE
    BigTrivialType const obj{
        0xF1359EA0221A3749U,
        0x51314BA1F17BCD21U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0xA1B2C3D4E5F60718U,
        0x0918273645546372U};
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret    = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<BigTrivialType>(msg);
    MessagePayloadBuilder::deallocate(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.c, storedObj.c);
}

TEST_F(TestMessagePayloadBuilder, TestSmallNonTrivialType)
{
    // ARRANGE
    SmallNonTrivialType obj{};
    obj.data.push_back(0x31U);
    obj.data.push_back(0x27U);
    obj.data.push_back(0x99U);
    obj.data.push_back(0x50U);
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj
        = MessagePayloadBuilder::getInstance().readPayload<SmallNonTrivialType>(msg);
    MessagePayloadBuilder::deallocate(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_EQ(obj.data, storedObj.data);
}

TEST_F(TestMessagePayloadBuilder, BigNonTrivialType)
{
    // ARRANGE
    BigNonTrivialType obj{};
    obj.a = 0xF131125CU;
    obj.b = 0x5DC09EA0U;
    for (size_t i = 0x00U; i < obj.data.max_size(); i++)
    {
        obj.data.push_back(i);
    }
    Message msg = makeEmptyMessage();

    // ACT
    core::HRESULT ret    = MessagePayloadBuilder::getInstance().allocate(obj, msg);
    auto const storedObj = MessagePayloadBuilder::getInstance().readPayload<BigNonTrivialType>(msg);
    MessagePayloadBuilder::deallocate(msg);

    // ASSERT
    EXPECT_EQ(ret, core::HRESULT::Ok);
    EXPECT_EQ(obj.a, storedObj.a);
    EXPECT_EQ(obj.b, storedObj.b);
    EXPECT_EQ(obj.data, storedObj.data);
}

TEST_F(TestMessagePayloadBuilder, TestBigTrivialTypeWithMultipleReferenceCounter)
{
    // ARRANGE
    BigTrivialType const obj{
        0xF1359EA0221A3749U,
        0x51314BA1F17BCD21U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0xA1B2C3D4E5F60718U,
        0x0918273645546372U};
    Message msg                      = Message::createEvent(123U, 128U, 1U, 0U);
    uint8_t const numberOfReferences = 5U;

    // ACT
    EXPECT_EQ(
        MessagePayloadBuilder::getInstance().allocate(obj, msg, numberOfReferences),
        core::HRESULT::Ok);

    // ASSERT
    for (size_t readings = 0U; readings < numberOfReferences; readings++)
    {
        EXPECT_TRUE(getAllocatorImpl().isAllocatorPoolFull(AllocatorImpl::PoolId::Pool128));
        auto const storedObj
            = MessagePayloadBuilder::getInstance().readPayload<BigTrivialType>(msg);
        EXPECT_EQ(obj.a, storedObj.a);
        EXPECT_EQ(obj.b, storedObj.b);
        EXPECT_EQ(obj.c, storedObj.c);
        MessagePayloadBuilder::deallocate(msg);
    }
    EXPECT_FALSE(getAllocatorImpl().isAllocatorPoolFull(AllocatorImpl::PoolId::Pool128));
}

TEST_F(TestMessagePayloadBuilder, TestBigNonTrivialTypeWithMultipleReferenceCounter)
{
    // ARRANGE
    BigNonTrivialType obj{};
    obj.a = 0xF131125CU;
    obj.b = 0x5DC09EA0U;
    for (size_t i = 0x00U; i < obj.data.max_size(); i++)
    {
        obj.data.push_back(i);
    }
    Message msg                      = Message::createEvent(123U, 128U, 1U, 0U);
    uint8_t const numberOfReferences = 5U;

    // ACT
    EXPECT_EQ(
        MessagePayloadBuilder::getInstance().allocate(obj, msg, numberOfReferences),
        core::HRESULT::Ok);

    // ASSERT
    for (size_t readings = 0U; readings < numberOfReferences; readings++)
    {
        EXPECT_TRUE(getAllocatorImpl().isAllocatorPoolFull(AllocatorImpl::PoolId::Pool128));
        auto const storedObj
            = MessagePayloadBuilder::getInstance().readPayload<BigNonTrivialType>(msg);
        EXPECT_EQ(obj.a, storedObj.a);
        EXPECT_EQ(obj.b, storedObj.b);
        EXPECT_EQ(obj.data, storedObj.data);
        MessagePayloadBuilder::deallocate(msg);
    }
    EXPECT_FALSE(getAllocatorImpl().isAllocatorPoolFull(AllocatorImpl::PoolId::Pool128));
}

TEST_F(TestMessagePayloadBuilder, TestFailedAllocationForTrivialType)
{
    // ARRANGE
    BigTrivialType const obj{
        0xF1359EA0221A3749U,
        0x51314BA1F17BCD21U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0x1289CEA256BD29A4U,
        0xA1B2C3D4E5F60718U,
        0x0918273645546372U};
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

TEST_F(TestMessagePayloadBuilder, TestFailedAllocationForNonTrivialType)
{
    // ARRANGE
    BigNonTrivialType obj{};
    obj.a = 0xF131125CU;
    obj.b = 0x5DC09EA0U;
    for (size_t i = 0x00U; i < obj.data.max_size(); i++)
    {
        obj.data.push_back(i);
    }
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
        static_cast<uint32_t>(BigNonTrivialType::AllocationPolicy::getNeededSize(obj)));
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
