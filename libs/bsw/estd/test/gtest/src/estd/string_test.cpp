// Copyright 2024 Accenture.

// Include class we are testing first to check for include dependencies
#include "estd/string.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(String, strnlen)
{
    ASSERT_EQ(12U, ::estd::strnlen("Hello world!", 15));
    ASSERT_EQ(10U, ::estd::strnlen("Hello world!", 10));
    char s[] = {'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'};
    ASSERT_EQ(12U, ::estd::strnlen(s, 15));
    ASSERT_EQ(10U, ::estd::strnlen(s, 10));
}

TEST(String, Size)
{
    ::estd::declare::string<10> s;
    ASSERT_EQ(0U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, ConstructorFromConstChar)
{
    ::estd::declare::string<10> s("Hello");
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, AssignmentConstChar)
{
    ::estd::declare::string<10> s;
    s = "Hello";
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, AssignmentString)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s = s1;
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, AssignmentChar)
{
    ::estd::declare::string<10> s;

    s = 'a';
    ASSERT_EQ('a', s[0]);
    ASSERT_EQ('\0', s[1]);
    ASSERT_EQ(1U, s.size());
}

TEST(String, OperatorAppendConstChar)
{
    ::estd::declare::string<10> s;
    s += "Hello";
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, OperatorAppendString)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s += s1;
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, OperatorAppendChar)
{
    ::estd::declare::string<10> s;

    s += 'a';
    ASSERT_EQ('a', s[0]);
    ASSERT_EQ('\0', s[1]);
    ASSERT_EQ(1U, s.size());
}

TEST(String, CStr)
{
    ::estd::declare::string<10> s("Hello");
    char const* data = s.c_str();

    ASSERT_EQ(data, s.data());

    ASSERT_EQ('H', data[0]);
    ASSERT_EQ('e', data[1]);
    ASSERT_EQ('\0', data[5]);
}

TEST(String, Empty)
{
    ::estd::declare::string<10> s;
    ASSERT_TRUE(s.empty());

    s = "Hello";
    ASSERT_FALSE(s.empty());
}

TEST(String, Full)
{
    ::estd::declare::string<10> s;
    ASSERT_FALSE(s.full());

    s = "123456789";
    ASSERT_TRUE(s.full());
    ASSERT_EQ('\0', s[9]);
}

TEST(String, AppendConstCharNBytes)
{
    ::estd::declare::string<10> s;
    s.append("HelloWorld", 5);

    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, AppendSubStringAll)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s.append(s1, 2); // llo
    ASSERT_EQ(3U, s.size());
    ASSERT_EQ('l', s[0]);
    ASSERT_EQ('o', s[2]);
    ASSERT_EQ('\0', s[3]);
}

TEST(String, AppendSubString)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s.append(s1, 1, 2); // el
    ASSERT_EQ(2U, s.size());
    ASSERT_EQ('e', s[0]);
    ASSERT_EQ('l', s[1]);
    ASSERT_EQ('\0', s[2]);
}

TEST(String, AppendSubStringTooBig)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s.append(s1, 2, 6); // llo
    ASSERT_EQ(3U, s.size());
    ASSERT_EQ('l', s[0]);
    ASSERT_EQ('l', s[1]);
    ASSERT_EQ('o', s[2]);
    ASSERT_EQ('\0', s[3]);
}

TEST(String, AppendFill)
{
    ::estd::declare::string<10> s;

    s.append(5U, 'c');
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ('c', s[0]);
    ASSERT_EQ('c', s[1]);
    ASSERT_EQ('c', s[2]);
    ASSERT_EQ('c', s[3]);
    ASSERT_EQ('c', s[4]);
    ASSERT_EQ('\0', s[5]);
}

TEST(String, AssignConstCharNBytes)
{
    ::estd::declare::string<10> s;
    s.assign("HelloWorld", 5);

    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(String, AssignSubStringAll)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s.assign(s1, 2); // llo
    ASSERT_EQ(3U, s.size());
    ASSERT_EQ('l', s[0]);
    ASSERT_EQ('o', s[2]);
    ASSERT_EQ('\0', s[3]);
}

TEST(String, AssignSubString)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s.assign(s1, 1, 2); // el
    ASSERT_EQ(2U, s.size());
    ASSERT_EQ('e', s[0]);
    ASSERT_EQ('l', s[1]);
    ASSERT_EQ('\0', s[2]);
}

TEST(String, AssignSubStringTooBig)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("Hello");

    s.assign(s1, 2, 6); // llo
    ASSERT_EQ(3U, s.size());
    ASSERT_EQ('l', s[0]);
    ASSERT_EQ('l', s[1]);
    ASSERT_EQ('o', s[2]);
    ASSERT_EQ('\0', s[3]);
}

TEST(String, Equal)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Hello");

    ASSERT_EQ("Hello", s);
    ASSERT_EQ(s, "Hello");
    ASSERT_EQ(s, s1);
}

TEST(String, NotEqual)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Hello");

    ASSERT_FALSE("Hello" != s);
    ASSERT_FALSE(s != "Hello");
    ASSERT_FALSE(s != s1);

    s += "a";
    ASSERT_TRUE(s != s1);
}

TEST(String, Less)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Hello");

    ASSERT_FALSE(s < "Hello");
    ASSERT_FALSE("Hello" < s);
    ASSERT_FALSE(s < s1);

    s1 += "a";
    ASSERT_TRUE(s < s1);
}

TEST(String, LessEqual)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Hello");

    ASSERT_TRUE(s <= "Hello");
    ASSERT_TRUE("Hello" <= s);
    ASSERT_TRUE(s <= s1);
}

TEST(String, Greater)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Hello");

    ASSERT_FALSE(s > "Hello");
    ASSERT_FALSE("Hello" > s);
    ASSERT_FALSE(s > s1);

    s += "a";
    ASSERT_TRUE(s > s1);
}

TEST(String, GreaterEqual)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Hello");

    ASSERT_TRUE(s >= "Hello");
    ASSERT_TRUE("Hello" >= s);
    ASSERT_TRUE(s >= s1);
}

TEST(String, FillConstructor)
{
    ::estd::declare::string<10> s(9U, 'c');
    ASSERT_EQ(9U, s.size());
    for (int32_t i = 0; i < 9; ++i)
    {
        ASSERT_EQ('c', s[i]) << i;
    }
}

TEST(String, ResizeNull)
{
    ::estd::declare::string<10> s;
    ASSERT_EQ(0U, s.size());

    s.resize(5);
    ASSERT_EQ(5U, s.size());
    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ('\0', s[i]);
    }
    ASSERT_EQ('\0', s[5]);
}

TEST(String, ResizeChar)
{
    ::estd::declare::string<10> s;
    ASSERT_EQ(0U, s.size());

    s.resize(5, 'c');
    ASSERT_EQ(5U, s.size());
    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ('c', s[i]);
    }
    ASSERT_EQ('\0', s[5]);
}

TEST(String, ResizeSame)
{
    ::estd::declare::string<10> s("Hello");
    ASSERT_EQ(5U, s.size());

    s.resize(5, 'c');
    ASSERT_EQ(5U, s.size());

    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('l', s[2]);
    ASSERT_EQ('l', s[3]);
    ASSERT_EQ('o', s[4]);
    ASSERT_EQ('\0', s[5]);
}

TEST(String, ResizeSmaller)
{
    ::estd::declare::string<10> s("Hello");
    ASSERT_EQ(5U, s.size());

    s.resize(3, 'c');
    ASSERT_EQ(3U, s.size());

    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('l', s[2]);
    ASSERT_EQ('\0', s[3]);
}

TEST(String, ResizeTooMuch)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::string<10> s;

    ASSERT_THROW({ s.resize(10, 'c'); }, ::estd::assert_exception);
}

TEST(String, DeclareAssignFromBase)
{
    ::estd::declare::string<10> s;
    ::estd::declare::string<10> s1("hello");

    ::estd::string& base = s1;
    s                    = base;
    ASSERT_EQ("hello", s);
}

TEST(String, DeclareConstructFromBase)
{
    ::estd::declare::string<10> s1("hello");

    ::estd::string& base = s1;

    ::estd::declare::string<10> s(base);
    ASSERT_EQ("hello", s);
}

TEST(String, DeclareConstructFromDeclare)
{
    ::estd::declare::string<10> s1("hello");

    ::estd::declare::string<10> s(s1);
    ASSERT_EQ("hello", s);
}

TEST(String, Length)
{
    ::estd::declare::string<10> s("Hello");
    ASSERT_EQ(5U, s.length());
    ASSERT_EQ(5U, s.size());
}

TEST(String, EqualityDifferentSize)
{
    ::estd::declare::string<10> s("Hello");
    ASSERT_FALSE(s == "ABC");
    ASSERT_TRUE(s == "Hello");
}

TEST(String, AssignConstChar)
{
    ::estd::declare::string<10> s("Hello");

    s.assign("ABC");
    ASSERT_EQ("ABC", s);
}

TEST(String, AppendConstCharNJustEnoughSpace)
{
    ::estd::declare::string<10> s("Hello");

    s.append("1234", 4);
    ASSERT_EQ("Hello1234", s);
}

TEST(String, AppendConstCharNNotEnoughSpace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::string<10> s("Hello");

    ASSERT_THROW({ s.append("12345", 5); }, ::estd::assert_exception);
}

TEST(String, AppendCharNJustEnoughSpace)
{
    ::estd::declare::string<10> s("Hello");

    s.append(4, '4');
    ASSERT_EQ("Hello4444", s);
    ASSERT_EQ('\0', s[9]);
}

TEST(String, AppendCharNNotEnoughSpace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::string<10> s("Hello");

    ASSERT_THROW({ s.append(5, '5'); }, ::estd::assert_exception);
}

TEST(String, AppendSubstringNotEnoughSpace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::string<10> s("Hello");

    ::estd::declare::string<10> s1("HelloWorl");
    ASSERT_THROW({ s.append(s1, 1, 5); }, ::estd::assert_exception);
}

TEST(String, Clear)
{
    ::estd::declare::string<10> s("Hello");

    s.clear();
    ASSERT_EQ("", s);
    ASSERT_EQ(0U, s.size());
}

TEST(String, At)
{
    ::estd::declare::string<10> s("Hello");
    ASSERT_EQ('H', s.at(0));
    ASSERT_EQ('\0', s.at(5));
}

TEST(String, ConstAt)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::string const& cs = s;

    ASSERT_EQ('H', cs.at(0));
    ASSERT_EQ('\0', cs.at(5));
}

TEST(String, AtOutOfBounds)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");
    ASSERT_EQ('H', s.at(0));
    ASSERT_EQ('\0', s.at(5));
    ASSERT_THROW({ s.at(6); }, ::estd::assert_exception);
}

TEST(String, ConstAtOutOfBounds)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");
    ::estd::string const& cs = s;

    ASSERT_EQ('H', cs.at(0));
    ASSERT_EQ('\0', cs.at(5));
    ASSERT_THROW({ cs.at(6); }, ::estd::assert_exception);
}

TEST(String, PlusEqualCharOutOfBounds)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("123456789");
    ASSERT_THROW({ s += 'c'; }, ::estd::assert_exception);
}

TEST(String, EqualCharOutOfBounds)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<1> s;
    ASSERT_THROW({ s = 'c'; }, ::estd::assert_exception);
}

TEST(String, EqualConstCharOutOfBounds)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<2> s;
    ASSERT_THROW({ s = "ab"; }, ::estd::assert_exception);
}

TEST(String, SelfAssignment)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::string& other = s;

    s = other;
    ASSERT_EQ("Hello", s);
}

TEST(String, AssignmentTooBig)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<12> s1("HelloWorld");

    try
    {
        s = s1;
        FAIL();
    }
    catch (::estd::assert_exception const& e)
    {}
}

TEST(String, AssignmentConstCharTooBig)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");

    try
    {
        s = "HelloWorld";
        FAIL();
    }
    catch (::estd::assert_exception const& e)
    {}
}

TEST(String, AssignStringSame)
{
    ::estd::declare::string<10> s("Hello");

    s.assign(s);
    ASSERT_EQ("Hello", s);
}

TEST(String, AssignStringNotSame)
{
    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("Other");

    s.assign(s1);
    ASSERT_EQ("Other", s);
}

TEST(String, AssignSubStringSame)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");

    ASSERT_THROW({ s.assign(s, 1, 2); }, ::estd::assert_exception);
}

TEST(String, AppendNotEnoughSpace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");
    ::estd::declare::string<10> s1("World");

    ASSERT_THROW({ s.append(s1); }, ::estd::assert_exception);
}

TEST(String, AppendSame)
{
    ::estd::declare::string<10> s("1234");

    s.append(s);
    ASSERT_EQ("12341234", s);
}

TEST(Test, AppendConstCharNotEnoughSpace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::string<10> s("Hello");

    ASSERT_THROW({ s.append("World"); }, ::estd::assert_exception);
}

/**
 * \desc
 * This test verifies that a simple conversion of a char of a number to a uint_8t with base 10
 * works.
 */
TEST(StrtonumTest, normal_conversion_uint8_t_base_10)
{
    uint8_t const bytes[] = {'1'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 1U);
}

/**
 * \desc
 * This test verifies that a simple conversion of a string from make_str works.
 */
TEST(StrtonumTest, normal_conversion_make_str)
{
    auto const bytes = "7";
    auto const res   = ::estd::strtonum<uint8_t>(::estd::make_str("7"), 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '" << bytes
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 7U);
}

/**
 * \desc
 * This test verifies that a simple conversion of a string from an ::estd::string works.
 */
TEST(StrtonumTest, normal_conversion_estd_string)
{
    ::estd::declare::string<2> numberString("6");
    auto const res = ::estd::strtonum<uint8_t>(numberString, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '" << numberString.c_str()
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 6U);
}

/**
 * \desc
 * This test verifies that a simple conversion of a string from an normal c_str works.
 */
TEST(StrtonumTest, normal_conversion_c_string)
{
    auto const bytes = "5";
    auto const res   = ::estd::strtonum<uint8_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '" << bytes
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 5U);
}

/**
 * \desc
 * This test verifies that a conversion of a char is converted to base 10 if the base is not
 * specified works.
 */
TEST(StrtonumTest, normal_conversion_uint8_t_without_base)
{
    uint8_t const bytes[] = {'1', '1'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 11U);
}

/**
 * \desc
 * This test verifies that a simple conversion of a 0 result in 0.
 */
TEST(StrtonumTest, normal_conversion_of_zero)
{
    uint8_t const bytes[] = {'0'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0U);
}

/**
 * \desc
 * This test verifies that a simple conversion of 0s result in 0.
 */
TEST(StrtonumTest, normal_conversion_of_multiple_zeros)
{
    uint8_t const bytes[] = {'0', '0', '0', '0'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0U);
}

/**
 * \desc
 * This test verifies that a simple conversion of a string with a plus sign works.
 */
TEST(StrtonumTest, normal_conversion_plus_sign)
{
    uint8_t const bytes[] = {'+', '1', '2'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 12U);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number with leading 0s to a uint_8t
 * with base 10 works.
 */
TEST(StrtonumTest, conversion_starting_0s_uint8_t_base_10)
{
    uint8_t const bytes[] = {'0', '0', '0', '9'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 9U);
}

/**
 * \desc
 * This test verifies that a simple conversion of a char of a number to a uint_8t with base 16
 * works.
 */
TEST(StrtonumTest, normal_conversion_uint8_t_base_16)
{
    uint8_t const bytes[] = {'F'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 16);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0xFU);
}

/**
 * \desc
 * This test verifies that a simple conversion of an array of chars of a lower case hex number
 * with prefix "0x" to a uint_8t with base 16 works.
 */
TEST(StrtonumTest, normal_conversion_uint8_t_0x_prefix_base_16)
{
    uint8_t const bytes[] = {'0', 'x', 'F'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 16);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0xFU);
}

/**
 * \desc
 * This test verifies that a simple conversion of an array of chars of a lower case hex number with
 * prefix "0X" to a uint_8t with base 16 works.
 */
TEST(StrtonumTest, normal_conversion_uint8_t_0X_prefix_base_16)
{
    uint8_t const bytes[] = {'0', 'X', 'F'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 16);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0xFU);
}

/**
 * \desc
 * This test verifies that a simple conversion of a char of a upper case hex number to a uint_8t
 * with base 16 works.
 */
TEST(StrtonumTest, small_letter_conversion_uint8_t_base_16)
{
    uint8_t const bytes[] = {'f'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 16);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0xFU);
}

/**
 * \desc
 * This test verifies that a conversion of an array of chars of a octal number with prefix
 * "0" to a uint_8t with base 16 works.
 */
TEST(StrtonumTest, normal_conversion_uint8_t_0_prefix_base_8)
{
    uint8_t const bytes[] = {'0', '7', '7'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 8);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 077U);
}

/**
 * \desc
 * This test verifies that a conversion of an array of chars of a number to a uint_64t with
 * base 10 works.
 */
TEST(StrtonumTest, normal_conversion_uint64_t_base_10)
{
    uint8_t const bytes[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 123456789U);
}

/**
 * \desc
 * This test verifies that a conversion of an array of chars of a number to a uint_64t with
 * base 16 works.
 */
TEST(StrtonumTest, normal_conversion_uint64_t_base_16)
{
    uint8_t const bytes[]
        = {'f', 'f', 'f', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F'};
    auto const res = ::estd::strtonum<uint64_t>(bytes, 16);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0xFFFFFFFFFFFFFFFFU);
}

/**
 * \desc
 * This test verifies that a simple conversion of chars of a negative number to a int with base 10
 * works.
 */
TEST(StrtonumTest, normal_conversion_int_negative_number)
{
    uint8_t const bytes[] = {'-', '1'};
    auto const res        = ::estd::strtonum<int>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), -1);
}

/**
 * \desc
 * This test verifies that a conversion of chars with more than one signs fails.
 */
TEST(StrtonumTest, failed_multiple_signs)
{
    uint8_t const bytes[] = {'-', '+', 'A', '9'};
    auto const res        = ::estd::strtonum<int8_t>(bytes, 16);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to int8_t as: " << static_cast<int>(res.get());
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a negative number to a int32_t with base
 * 10 works.
 */
TEST(StrtonumTest, int32_t_normal_conversion_min)
{
    uint8_t const bytes[] = {'-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
    auto const res        = ::estd::strtonum<int32_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), std::numeric_limits<int32_t>::min());
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number bigger than max of int32_t to a
 * int32_t with base 10 returns a result with an overflow error.
 */
TEST(StrtonumTest, int32_t_overflow_conversion)
{
    uint8_t const bytes[] = {'2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
    auto const res        = ::estd::strtonum<int32_t>(bytes, 10);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to int32_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a negative number smaller than min of int32_t to
 * a int32_t with base 10 returns a result with an overflow error.
 */
TEST(StrtonumTest, int32_t_negative_overflow_conversion)
{
    uint8_t const bytes[] = {'-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '9'};
    auto const res        = ::estd::strtonum<int32_t>(bytes, 10);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to int32_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a negative number equal to min of int64_t to a
 * int64_t with base 10 works.
 */
TEST(StrtonumTest, int64_t_normal_conversion_min)
{
    uint8_t const bytes[] = {'-', '9', '2', '2', '3', '3', '7', '2', '0', '3',
                             '6', '8', '5', '4', '7', '7', '5', '8', '0', '8'};
    auto const res        = ::estd::strtonum<int64_t>(bytes, 10);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), std::numeric_limits<int64_t>::min());
}

/**
 * \desc
 * This test verifies that a conversion of chars in base 2 of a number equal to max of uint64_t to a
 * uint64_t with base 2 works.
 */
TEST(StrtonumTest, base_2_conversion)
{
    uint8_t const bytes[64]
        = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
           '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
           '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1',
           '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
    auto const res = ::estd::strtonum<uint64_t>(bytes, 2);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0xFFFFFFFFFFFFFFFF);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number bigger than max of int64_t to a
 * int64_t with base 10 returns a result with an overflow error.
 */
TEST(StrtonumTest, int64_t_overflow_conversion)
{
    // clang-format off
    uint8_t const bytes[] = {'9','2','2','3','3','7','2','0','3','6','8','5','4','7','7','5','8','0','8'};
    // clang-format on
    auto const res = ::estd::strtonum<int64_t>(bytes, 10);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to int64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a negative number slightly smaller than min of
 * int64_t to a int64_t with base 10 returns a result with an overflow error.
 */
TEST(StrtonumTest, int64_t_negative_overflow_conversion)
{
    uint8_t const bytes[] = {'-', '9', '2', '2', '3', '3', '7', '2', '0', '3',
                             '6', '8', '5', '4', '7', '7', '5', '8', '0', '9'};
    auto const res        = ::estd::strtonum<int64_t>(bytes, 10);
    ASSERT_FALSE(res) << "Parsed '-9223372036854775809' to int64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a negative number much smaller than min of
 * int64_t to a int64_t with base 10 returns a result with an overflow error.
 */
TEST(StrtonumTest, int32_t_negative_big_overflow_conversion)
{
    uint8_t const bytes[] = {'-', '9', '2', '2', '3', '3', '7', '2', '0', '3',
                             '6', '8', '5', '4', '7', '7', '5', '8', '0', '9'};
    auto const res        = ::estd::strtonum<int32_t>(bytes, 10);
    ASSERT_FALSE(res) << "Parsed '-9223372036854775809' to int32_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a negative number to a uint64_t returns a
 * result with an invalid token error.
 */
TEST(StrtonumTest, failed_negative_to_unsigned)
{
    uint8_t const bytes[] = {'-', '9', '2', '2', '3', '3', '7', '2', '0', '3',
                             '6', '8', '5', '4', '7', '7', '5', '8', '0', '9'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 16);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number bigger than max of uint8_t to a
 * uint8_t with base 16 returns a result with an overflow error.
 */
TEST(StrtonumTest, uint8_t_overflow)
{
    uint8_t const bytes[] = {'1', '2', '3'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 16);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint8_t as: " << static_cast<int>(res.get());
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number bigger than max of uint64_t to a
 * uint64_t with base 10 returns a result with an overflow error.
 */
TEST(StrtonumTest, uint64_t_base_10_overflow_conversion)
{
    for (uint8_t i = '6'; i <= '9'; i++)
    {
        uint8_t const bytes[] = {'1', '8', '4', '4', '6', '7', '4', '4', '0', '7',
                                 '3', '7', '0', '9', '5', '5', '1', '6', '1', i};
        auto const res        = ::estd::strtonum<uint64_t>(bytes);
        ASSERT_FALSE(res) << "Parsed "
                          << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                          << " to uint64_t as: " << res.get();
        EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
    }
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number bigger than max of uint64_t to a
 * uint64_t with base 16 returns a result with an overflow error.
 */
TEST(StrtonumTest, uint64_t_base_16_overflow_conversion)
{
    uint8_t const bytes[] = {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E',
                             'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 16);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of chars of a number bigger than max of uint64_t to a
 * uint64_t with base 15 returns a result with an overflow error.
 */
TEST(StrtonumTest, uint64_t_base_15_overflow_conversion)
{
    // 15 base overflow == 2C1D56B648C6CD111
    uint8_t bytes[]
        = {'2', 'C', '1', 'D', '5', '6', 'B', '6', '4', '8', 'C', '6', 'C', 'D', '1', '1', '1'};
    for (char number = '2'; number <= '9'; number++)
    {
        for (char placeFromBack = 1; placeFromBack <= 3; placeFromBack++)
        {
            {
                bytes[sizeof(bytes) - placeFromBack] = number;
                auto const res                       = ::estd::strtonum<uint64_t>(bytes, 15);
                ASSERT_FALSE(res) << "Parsed "
                                  << std::string(
                                         reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                  << " to uint64_t as: " << res.get();
                EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
            }
        }
    }
}

/**
 * \desc
 * This test loops through a valid set base 9 chars and converts it to
 * uint8_t with base 9 and all returns a result with an overflow error,
 * then it loops through a set of chars which is invalid base 9 chars and converts it to
 * uint8_t with base 9 and all returns a result with an invalid token error.
 */
TEST(StrtonumTest, uint8_t_base_9_fail_loop)
{
    // 313 in base 9 is 0xFF
    for (size_t i = 314; i < 999999; i++)
    {
        auto string = std::to_string(i);
        if (string.find_first_of('9') == static_cast<size_t>(-1))
        {
            auto const bytes = ::estd::make_str(string.c_str());
            auto const res   = ::estd::strtonum<uint8_t>(bytes, 9);
            ASSERT_FALSE(res) << "Parsed " << string
                              << " to uint8_t as: " << static_cast<int>(res.get());
            EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
        }
    }
    uint8_t byte[] = {0};
    for (uint8_t i = 1; i != 0; i++)
    {
        auto const res = ::estd::strtonum<uint8_t>(byte, 36);
        ASSERT_FALSE(res) << "Parsed " << byte << " to uint8_t as: " << static_cast<int>(res.get());
        EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
        if (i == '0')
        {
            i += 10;
        }
        else if (i == 'A' || i == 'a')
        {
            i += 26;
        }
        byte[0] = i;
    }
}

/**
 * \desc
 * This test loops through a valid set base 11 chars and converts it to
 * uint16_t with base 11, verifying they are all under an upper bound.
 */
TEST(StrtonumTest, uint16_t_succeed_loop)
{
    int32_t next = -1;
    // 45268 in base 11 is 0xFFFF
    for (size_t i = 0; i <= 45268; i++)
    {
        auto const string = std::to_string(i);
        auto const bytes  = ::estd::make_str(string.c_str());
        auto const res    = ::estd::strtonum<uint16_t>(bytes, 11);
        ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '" << string
                                     << "': " << static_cast<int>(res.error());
        next++;
        if (res.get() != next)
        {
            // Upper bound ( lim (10...)_base11 / ( (9... )_base11 + 1) = 1.1... = 10/9 )
            // Example:
            // https://www.wolframalpha.com/input?i2d=true&i=Divide%5B%5C%2840%291000000000000+base+11%5C%2841%29%2C99999999999A+base+11%5D

            ASSERT_LE(res.get(), (next) * 10 / 9);
            next = res.get();
        }
    }
}

/**
 * \desc
 * This test verifies that a conversion of a set of invalid tokens in base 16 to a
 * uint64_t with base 16 returns a result with an invalid error.
 */
TEST(StrtonumTest, failed_conversion_invalid_tokens)
{
    // All invalid tokens in base 16 == [0:0xFF]=0x100 - [0-9]=10 - [A-Fa-f]=2*6
    constexpr auto length = 0x100 - 10 - 2 * 6;
    uint8_t bytes[length];
    size_t j = 0;
    for (size_t i = 0; i < 0x100; i++)
    {
        if (i == '0')
        {
            i += 10;
        }
        else if (i == 'A' || i == 'a')
        {
            i += 6;
        }
        bytes[j] = i;
        j++;
    }

    for (size_t i = 0; i < length; i++)
    {
        uint8_t const byte[] = {bytes[i]};
        auto const res       = ::estd::strtonum<uint64_t>(byte, 16);
        ASSERT_FALSE(res) << "Parsed "
                          << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                          << " to uint64_t as: " << res.get();
        EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
    }
}

/**
 * \desc
 * This test verifies that a conversion of too many tokens results with an invalid token error.
 */
TEST(StrtonumTest, failed_conversion_too_long_token)
{
    uint8_t bytes[65];
    for (size_t i = 0; i < 65; i++)
    {
        bytes[i] = '1';
    }

    auto const res = ::estd::strtonum<uint64_t>(bytes, 2);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::Overflow);
}

/**
 * \desc
 * This test verifies that a conversion of too many tokens, but leading zeros, works.
 */
TEST(StrtonumTest, succeed_conversion_too_long_token_with_zeros)
{
    // Way too long token
    uint8_t bytes[127];
    for (size_t i = 0; i < 126; i++)
    {
        bytes[i] = '0';
    }
    bytes[126] = '1';

    auto const res = ::estd::strtonum<uint64_t>(bytes, 2);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 1U);
}

/**
 * \desc
 * This test verifies that a conversion of chars with 1 invalid token in base 2 to a
 * uint64_t with base 2 returns a result with an invalid error.
 */
TEST(StrtonumTest, partially_failed_conversion_base_2)
{
    uint8_t const bytes[] = {'1', '0', '0', '0', '2', '1', '0', '1'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 2);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of chars with 1 invalid token in base 10 to a
 * uint64_t with base 10 returns a result with an invalid error.
 */
TEST(StrtonumTest, partially_failed_conversion_base_10)
{
    uint8_t const bytes[] = {'1', '2', '3', 'a' - 1, '5', '6', '7', '8'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 10);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a simple conversion of a base 16 chars fails when converted to base 10
 * if the base is not specified.
 */
TEST(StrtonumTest, failed_conversion_without_base)
{
    uint8_t const bytes[] = {'1', 'F'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of a base 16 chars in a slice smaller than the buffer only
 * uses the size of the buffer for the conversion.
 */
TEST(StrtonumTest, smaller_slice_from_make_str)
{
    auto const bytes = ::estd::make_str("123F5678");
    auto s           = ::estd::slice<uint8_t const, 4>::from_pointer(bytes.data());
    auto const res   = ::estd::strtonum<uint64_t>(s, 16);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '" << s.data()
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0x123F);
}

/**
 * \desc
 * This test verifies that a conversion an empty array fails.
 */
TEST(StrtonumTest, empty_slice)
{
    auto const bytes = ::estd::make_str("");
    auto const res   = ::estd::strtonum<uint64_t>(bytes);
    ASSERT_FALSE(res) << "Parsed empty string as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of an array with only a valid minus sign fails.
 */
TEST(StrtonumTest, minus_sign_slice)
{
    auto const bytes = ::estd::make_str("-");
    auto const res   = ::estd::strtonum<uint64_t>(bytes);
    ASSERT_FALSE(res) << "Parsed minus sign as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of an array with only a valid plus sign fails.
 */
TEST(StrtonumTest, plus_sign_slice)
{
    uint8_t const bytes[] = {'+'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes);
    ASSERT_FALSE(res) << "Parsed plus sign as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of a zero slice from an array with only a valid sign fails.
 */
TEST(StrtonumTest, zero_length_plus_sign_slice)
{
    uint8_t const bytes[] = {'+'};
    auto s                = ::estd::slice<uint8_t const, 0>::from_pointer(bytes, 0);
    auto const res        = ::estd::strtonum<uint64_t>(s);
    ASSERT_FALSE(res) << "Parsed zero length array with plus sign as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidToken);
}

/**
 * \desc
 * This test verifies that a conversion of chars to a
 * uint64_t with too big base returns a result with an invalid base.
 */
TEST(StrtonumTest, invalid_base_too_big)
{
    uint8_t const bytes[] = {'1', '2', '3', 'F', '5', '6', '7', '8'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 64);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidBase);
}

/**
 * \desc
 * This test verifies that a conversion of chars to a
 * uint64_t with too small base returns a result with an invalid base.
 */
TEST(StrtonumTest, invalid_base_too_small)
{
    uint8_t const bytes[] = {'1', '2', '3', 'F', '5', '6', '7', '8'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 1);
    ASSERT_FALSE(res) << "Parsed "
                      << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                      << " to uint64_t as: " << res.get();
    EXPECT_EQ(res.error(), ::estd::ConvertError::InvalidBase);
}

/**
 * \desc
 * This test verifies that a conversion of base 10 chars is automatically converted to a base 10
 * number when using '0' as base.
 */
TEST(StrtonumTest, auto_base_10)
{
    uint8_t const bytes[] = {'1', '2', '3', '9', '5', '6', '7'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 0);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 1239567U);
}

/**
 * \desc
 * This test verifies that a conversion of base 16 chars with 0x prefix is automatically converted
 * to a base 16 number when using '0' as base.
 */
TEST(StrtonumTest, auto_base_16)
{
    uint8_t const bytes[] = {'0', 'x', '1', '2', '3', 'F', '5', '6'};
    auto const res        = ::estd::strtonum<uint64_t>(bytes, 0);
    ASSERT_TRUE(res.has_value()) << "Unexpected result error from string '"
                                 << std::string(reinterpret_cast<char const*>(bytes), sizeof(bytes))
                                 << "': " << static_cast<int>(res.error());
    EXPECT_EQ(res.get(), 0x123F56U);
}

} // namespace
