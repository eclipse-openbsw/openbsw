// Copyright 2024 Accenture.

#include "estd/typed_mem.h"

#include "estd/slice.h"

#include <gmock/gmock.h>

#include <array>

namespace
{
using namespace ::testing;

class TestData
{
    uint32_t _value;

public:
    static size_t constructorCalls;
    static size_t destructorCalls;

    static void resetStats()
    {
        constructorCalls = 0;
        destructorCalls  = 0;
    }

    TestData() : _value(0) { ++constructorCalls; }

    explicit TestData(uint32_t value) : _value(value) { ++constructorCalls; }

    ~TestData() { ++destructorCalls; }

    uint32_t get() const { return _value; }
};

size_t TestData::constructorCalls{0};
size_t TestData::destructorCalls{0};

class TypedMemTest : public ::testing::Test
{
public:
    void SetUp() override { TestData::resetStats(); }
};

/**
 * Tests the a default constructed typed_mem holds no value.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, constructor)
{
    ::estd::typed_mem<TestData> t;
    EXPECT_EQ(0, TestData::constructorCalls);
    EXPECT_FALSE(t.has_value());
}

/**
 * Verifies that accessing a typed_mem that has not been emplaced asserts.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, access_without_emplace_asserts)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::typed_mem<TestData> t;
    EXPECT_THROW({ (void)t->get(); }, ::estd::assert_exception);
}

/**
 * Verifies that emplace calls T's constructor and accessing the emplaced instance doesn't assert.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, access_after_emplace_does_not_assert)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::typed_mem<TestData> t;
    t.emplace();
    EXPECT_EQ(1, TestData::constructorCalls);
    EXPECT_NO_THROW({ (void)t->get(); });
}

/**
 * Verifies that calling emplace() on an instance of typed_mem<T> that already holds a value
 * asserts.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, calling_emplace_twice_asserts)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::typed_mem<TestData> t;
    t.emplace();
    EXPECT_THROW({ t.emplace(); }, ::estd::assert_exception);
}

/**
 * Verifies that calling emplace forwards the arguments to the constructor of T.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, emplace_forwards_args_to_constructor)
{
    ::estd::typed_mem<TestData> t;
    t.emplace(0x1234);
    EXPECT_EQ(1, TestData::constructorCalls);
    EXPECT_EQ(0x1234, t->get());
}

/**
 * Verifies that the default constructor of typed_mem doesn't clear the internal memory, except
 * for the pointer to the value data.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, default_constructor_does_not_zero_memory)
{
    using T = estd::typed_mem<TestData>;
    // Allocate a buffer to hold an instance of typed_mem<T>.
    alignas(T) std::array<uint8_t, sizeof(T)> buffer{};
    std::fill(buffer.begin(), buffer.end(), 0xFF);
    EXPECT_THAT(buffer, Each(Eq(0xFF)));
    // Construct and instance of T into that buffer using the default constructor.
    new (buffer.data()) T();
    // Create a view on the buffer that excludes the pointer which has been set to nullptr.
    estd::slice<uint8_t> internalBuffer(buffer);
    internalBuffer.trim(sizeof(T) - sizeof(std::nullptr_t));
    EXPECT_THAT(internalBuffer, Each(Eq(0xFF)));
}

/**
 * Verifies that the non default constructor of typed_mem does clear the internal memory.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, constructor_does_zero_memory)
{
    using T = estd::typed_mem<TestData>;
    // Allocate a buffer to hold an instance of typed_mem<T>.
    alignas(T) std::array<uint8_t, sizeof(T)> buffer{};
    std::fill(buffer.begin(), buffer.end(), 0xFF);
    EXPECT_THAT(buffer, Each(Eq(0xFF)));
    // Construct and instance of T into that buffer using the default constructor.
    new (buffer.data()) T(T::zero_data());
    // Create a view on the buffer that holds the internal memory for TestData.
    estd::slice<uint8_t> internalBuffer(buffer);
    internalBuffer.trim(sizeof(TestData));
    EXPECT_THAT(internalBuffer, Each(Eq(0x00)));
}

/**
 * Verifies that the destructor of typed_mem<T> doesn't call the destructor of the internally
 * constructed object.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, destructor_does_not_destroy_internal_instance)
{
    {
        EXPECT_EQ(0, TestData::constructorCalls);
        ::estd::typed_mem<TestData> t;
        EXPECT_EQ(0, TestData::constructorCalls);
        t.emplace();
        EXPECT_EQ(1, TestData::constructorCalls);
    }
    EXPECT_EQ(1, TestData::constructorCalls);
    EXPECT_EQ(0, TestData::destructorCalls);
}

/**
 * Verifies that calling destroy() calls the destructor of the emplaced object.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, destroy_calls_destructor)
{
    ::estd::typed_mem<TestData> t;
    t.emplace();
    EXPECT_TRUE(t.has_value());
    EXPECT_EQ(1, TestData::constructorCalls);
    // Calling destroy will call the destructor and set has_value to false.
    t.destroy();
    EXPECT_EQ(1, TestData::destructorCalls);
    EXPECT_FALSE(t.has_value());
}

/**
 * Verifies that calling destroy() asserts if emplace has not been called before.
 * \refs    SMD_estd_typed_mem
 */
TEST_F(TypedMemTest, destroy_asserts_without_value)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::typed_mem<TestData> t;
    EXPECT_THROW({ t.destroy(); }, ::estd::assert_exception);
}

} // namespace
