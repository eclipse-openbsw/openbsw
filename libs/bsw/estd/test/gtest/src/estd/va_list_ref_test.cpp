// Copyright 2024 Accenture.

// Include class we are testing first to check for include dependencies
#include "estd/va_list_ref.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

namespace
{
static char const* cptr = "a";

class VaListRefFixture : public ::testing::Test
{
protected:
    void testConstructor(int start, ...)
    {
        va_list ap;
        va_start(ap, start);
        ::estd::va_list_ref cut(ap);
        ASSERT_EQ(cptr, va_arg(cut.get(), char const*));
        ASSERT_EQ(2U, va_arg(cut.get(), uint32_t));
        ASSERT_EQ(45345ULL, va_arg(cut.get(), uint64_t));
        va_end(ap);
    }

    void testCopyConstructor(int start, ...)
    {
        va_list ap;
        va_start(ap, start);
        ::estd::va_list_ref ref(ap);
        ::estd::va_list_ref cut(ref);
        ASSERT_EQ(cptr, va_arg(cut.get(), char const*));
        ASSERT_EQ(2U, va_arg(cut.get(), uint32_t));
        ASSERT_EQ(45345ULL, va_arg(cut.get(), uint64_t));
        va_end(ap);
    }
};

TEST_F(VaListRefFixture, executeTests)
{
    testConstructor(1, cptr, 2, 45345ULL);
    testCopyConstructor(1, cptr, 2, 45345ULL);
}

TEST(VaListRefInternalDefault, executeTests)
{
    uint32_t t = 17U;
    ::estd::internal::_va_list_ref<uint32_t> cut(t);
    EXPECT_EQ(17U, cut.get());
    ::estd::internal::_va_list_ref<uint32_t> copy(cut);
    EXPECT_EQ(17U, copy.get());
}

TEST(VaListRefInternal, executeTests)
{
    using ArrayType = uint32_t[2];
    uint32_t t[2]   = {17U, 18U};
    ::estd::internal::_va_list_ref<ArrayType> cut(t);
    {
        ArrayType& value = cut.get();
        EXPECT_EQ(17U, value[0]);
        EXPECT_EQ(18U, value[1]);
    }
    ::estd::internal::_va_list_ref<ArrayType> copy(cut);
    {
        ArrayType& value = copy.get();
        EXPECT_EQ(17U, value[0]);
        EXPECT_EQ(18U, value[1]);
    }
}
} // namespace
