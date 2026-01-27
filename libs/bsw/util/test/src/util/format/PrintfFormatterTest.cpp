// Copyright 2024 Accenture.

#include "util/format/PrintfFormatter.h"

#include "util/format/StringWriter.h"
#include "util/stream/StringBufferOutputStream.h"
#include "util/string/ConstString.h"

#include <gtest/gtest.h>

#include <cstdarg>

namespace
{
using namespace ::util::format;
using namespace ::util::stream;
using namespace ::util::string;

#define expect

struct PrintfFormatterTest
: public ::testing::Test
, protected IPrintfArgumentReader
{
    enum Flags
    {
        IGNORE_POSITIVE = 0x01,
        IGNORE_NEGATIVE = 0x02,
        IGNORE_ZERO     = 0x04
    };

    ParamVariant const* readArgument(ParamDatatype /* datatype */) override { return nullptr; }

#if defined(__linux) && defined(__GNUC__) && __x86_64__
    // NOLINTNEXTLINE(cert-dcl50-cpp): va_list usage only for printing functionalities.
    std::string printf(char const* formatString, ...)
    {
        va_list ap;
        va_start(ap, formatString);
        std::string result = printfV(formatString, ap);
        va_end(ap);
        return result;
    }

    std::string printfV(char const* formatString, va_list ap)
    {
        char buffer[300];
        static_cast<void>(vsnprintf(buffer, sizeof(buffer), formatString, ap));
        return buffer;
    }
#endif
    // NOLINTNEXTLINE(cert-dcl50-cpp): va_list usage only for printing functionalities.
    std::string format(char const* formatString, ...)
    {
        va_list ap;
        va_start(ap, formatString);
        std::string result = formatV(formatString, ap);
        va_end(ap);
        return result;
    }

    std::string formatV(char const* formatString, va_list ap)
    {
        declare::StringBufferOutputStream<300> stream;
        PrintfFormatter formatter(stream);
        formatter.format(formatString, ap);
        return stream.getString();
    }

#if defined(__linux) && defined(__GNUC__) && __x86_64__
    template<class T>
    void checkPrintf(char const* formatString, T value)
    {
        std::string pf = printf(formatString, value);
        std::string f  = format(formatString, value);
        ASSERT_EQ(pf, f);
    }

    template<class T1, class T2>
    void checkPrintf(char const* formatString, T1 value1, T2 value2)
    {
        std::string pf = printf(formatString, value1, value2);
        std::string f  = format(formatString, value1, value2);
        ASSERT_EQ(pf, f);
    }
#else
    template<class T>
    void checkPrintf(char const*, T)
    {}

    template<class T1, class T2>
    void checkPrintf(char const*, T1, T2)
    {}
#endif

    // NOLINTNEXTLINE(cert-dcl50-cpp): va_list usage only for printing functionalities.
    void expectPrintf(char const* pExpected, char const* formatString, ...)
    {
        va_list ap;
        va_start(ap, formatString);
        expectPrintfV(pExpected, formatString, ap);
        va_end(ap);
    }

    void expectPrintfV(char const* pExpected, char const* formatString, va_list ap)
    {
        ASSERT_EQ(pExpected, formatV(formatString, ap));
    }

    template<class T>
    void expectAndCheckPrintf(char const* pExpected, char const* formatString, T value)
    {
        expectPrintf(pExpected, formatString, value);
        checkPrintf(formatString, value);
    }

    template<class T1, class T2>
    void expectAndCheckPrintf(char const* pExpected, char const* formatString, T1 value1, T2 value2)
    {
        expectPrintf(pExpected, formatString, value1, value2);
        checkPrintf(formatString, value1, value2);
    }

    template<class T>
    void
    expectAndCheckIntPrintf(bool check, char const* pExpected, char const* formatString, T value)
    {
        if (pExpected)
        {
            if (check)
            {
                expectAndCheckPrintf(pExpected, formatString, value);
            }
            else
            {
                expectPrintf(pExpected, formatString, value);
            }
        }
    }

    template<class T>
    void expectAndCheckIntPrintf(
        char const* formatString,
        T value,
        char const* pPositive,
        char const* pNegative,
        char const* pZero,
        uint8_t flags = 0)
    {
        expectAndCheckIntPrintf(!(flags & IGNORE_NEGATIVE), pNegative, formatString, -value);
        expectAndCheckIntPrintf(!(flags & IGNORE_ZERO), pZero, formatString, static_cast<T>(0));
        expectAndCheckIntPrintf(!(flags & IGNORE_POSITIVE), pPositive, formatString, value);
    }
};

TEST_F(PrintfFormatterTest, testDefaultFormats)
{
    expectAndCheckPrintf("  t", "%03c", 't');
    expectAndCheckPrintf("  t", "%3c", 't');
    expectAndCheckPrintf("         abc", "%012s", "abc");
    expectAndCheckPrintf("abcdefghijklmnop", "%12s", "abcdefghijklmnop");
    expectAndCheckPrintf("abcdefghijkl", "%.12s", "abcdefghijklmnop");
    expectAndCheckPrintf("abcdefghi", "%.12s", "abcdefghi");
    expectAndCheckPrintf("         abc", "%#12s", "abc");
    expectAndCheckIntPrintf<int32_t>("%d", 47, "47", "-47", "0");
    expectAndCheckIntPrintf<int32_t>("%i", 47, "47", "-47", "0");
    expectAndCheckIntPrintf<int32_t>("%Ld", 47, "47", "-47", "0", IGNORE_NEGATIVE);
    expectAndCheckIntPrintf<int64_t>(
        "%ld", 4234892348329347, "4234892348329347", "-4234892348329347", "0");
    expectAndCheckIntPrintf<int64_t>(
        "%lld", 4234892348329347, "4234892348329347", "-4234892348329347", "0");
    expectAndCheckIntPrintf<uint32_t>("% u", 47, "47", nullptr, "0");
    expectAndCheckIntPrintf<uint32_t>("%+u", 47, "47", nullptr, "0");
    expectAndCheckIntPrintf<int32_t>("% d", 47, " 47", "-47", " 0");
    expectAndCheckIntPrintf<int32_t>("% +d", 47, "+47", "-47", "+0");
    expectAndCheckIntPrintf<int32_t>("%+12d", 47, "         +47", "         -47", "          +0");
    expectAndCheckIntPrintf<int32_t>("%+012d", 47, "+00000000047", "-00000000047", "+00000000000");
    expectAndCheckIntPrintf<int32_t>("%-012d", 47, "47          ", "-47         ", "0           ");
    expectAndCheckIntPrintf<int32_t>(
        "%+012.4d", 47, "       +0047", "       -0047", "       +0000");
    expectAndCheckIntPrintf<int32_t>(
        "%-012.4d", 47, "0047        ", "-0047       ", "0000        ");
    expectAndCheckIntPrintf<uint32_t>("%-012.4u", 47, "0047        ", nullptr, "0000        ");
    expectAndCheckIntPrintf<int32_t>("%01d", 47, "47", "-47", "0");
    expectAndCheckIntPrintf<int32_t>("%+12.4d", 47, "       +0047", "       -0047", "       +0000");
    expectAndCheckIntPrintf<int16_t>("%+12hd", 47, "         +47", "         -47", "          +0");
    expectAndCheckIntPrintf<int64_t>(
        "%+16ld", 24329482243247, " +24329482243247", " -24329482243247", "              +0");
    expectAndCheckIntPrintf<int64_t>(
        "%+16lld", 24329482243247, " +24329482243247", " -24329482243247", "              +0");
    expectAndCheckIntPrintf<int64_t>(
        "%+16llu", 24329482243247, "  24329482243247", nullptr, "               0");
    expectAndCheckIntPrintf<int32_t>("%+0.4d", 47, "+0047", "-0047", "+0000");
    expectAndCheckIntPrintf<int32_t>("%+.4d", 47, "+0047", "-0047", "+0000");
    expectAndCheckIntPrintf<int32_t>("%+8.0d", 47, "     +47", "     -47", "       +");
    expectAndCheckIntPrintf<uint32_t>("%x", 0x1a3c5, "1a3c5", nullptr, "0");
    expectAndCheckIntPrintf<uint16_t>("%hx", 0x23c5, "23c5", nullptr, "0");
    expectAndCheckIntPrintf<uint64_t>("%lx", 0x1a3c5d7e9f0, "1a3c5d7e9f0", nullptr, "0");
    expectAndCheckIntPrintf<uint64_t>("%llx", 0x1a3c5d7e9f0, "1a3c5d7e9f0", nullptr, "0");
    expectAndCheckIntPrintf<uint32_t>(
        "%#+9x", 0x1a3c5, "  0x1a3c5", nullptr, "      0x0", IGNORE_ZERO);
    expectAndCheckIntPrintf<uint32_t>(
        "%#+09X", 0x1a3c5, "0X001A3C5", nullptr, "0X0000000", IGNORE_ZERO);
    expectAndCheckIntPrintf<uint32_t>("%o", 047, "47", nullptr, "0");
    expectAndCheckIntPrintf<uint16_t>("%ho", 047, "47", nullptr, "0");
    expectAndCheckIntPrintf<uint64_t>("%lo", 047234672362234, "47234672362234", nullptr, "0");
    expectAndCheckIntPrintf<uint64_t>("%llo", 047234672362234, "47234672362234", nullptr, "0");
    expectAndCheckIntPrintf<uint32_t>("%#+9o", 047, "      047", nullptr, "        0");
    expectAndCheckIntPrintf<uint32_t>("%#+09o", 047, "000000047", nullptr, "000000000");
    expectAndCheckPrintf(" 0x1a3c5", "%#*x", 8, 0x1a3c5);
    expectAndCheckPrintf("0047", "%#.*d", 4, 47);
    uintptr_t const ptr_val = 0x1a3c5U;
    expectAndCheckPrintf("0x1a3c5", "%#p", reinterpret_cast<char const*>(ptr_val));
    expectAndCheckPrintf("Test%", "Test%+# 9.0ll%", 12);
    expectAndCheckPrintf("Test%Test", "Test%%Test", 12);
}

TEST_F(PrintfFormatterTest, testExtendedFormats)
{
    expectPrintf("         abc", "%012S", ConstString("abc").plain_str());
    expectPrintf("abcdefghijklmnop", "%12S", ConstString("abcdefghijklmnop").plain_str());
    expectPrintf("abcdefghijkl", "%.12S", ConstString("abcdefghijklmnop").plain_str());
    expectPrintf("abcdefghi", "%.12S", ConstString("abcdefghi").plain_str());
    expectPrintf("         abc", "%#12S", ConstString("abc").plain_str());
}

TEST_F(PrintfFormatterTest, testPositionParam)
{
    uint32_t pos1 = 0;
    uint32_t pos2 = 0;
    ASSERT_EQ("abcdefgh", format("abcd%nefgh%n", &pos1, &pos2));
    ASSERT_EQ(4U, pos1);
    ASSERT_EQ(8U, pos2);
    format("abcd%nefgh", 0);
}

TEST_F(PrintfFormatterTest, testNullPtr)
{
    ASSERT_EQ("<NULL>", format("%s", 0));
    ASSERT_EQ("<NULL>", format("%S", 0));
    declare::StringBufferOutputStream<300> stream;
    PrintfFormatter formatter(stream);
    formatter.format("%*.*d", *this);
    ASSERT_EQ("<?>", std::string(stream.getString()));
}

TEST_F(PrintfFormatterTest, testFormatWithEllipsis)
{
    declare::StringBufferOutputStream<300> stream;
    PrintfFormatter formatter(stream);
    formatter.format("%d %s", 17, "abc");
    ASSERT_EQ("17 abc", std::string(stream.getString()));
}

TEST_F(PrintfFormatterTest, testFormatParamWithInvalidValues)
{
    declare::StringBufferOutputStream<300> stream;
    {
        PrintfFormatter formatter(stream);
        {
            ParamInfo paramInfo = {static_cast<ParamType>(80), 0, 10, ParamDatatype::UINT16, 0, 0};
            formatter.formatParam(paramInfo, ParamVariant());
        }
        stream.write('.');
        {
            ParamInfo paramInfo = {ParamType::INT, 0, 10, ParamDatatype::COUNT, 0, 0};
            formatter.formatParam(paramInfo, ParamVariant());
        }
        stream.write('.');
        {
            ParamInfo paramInfo = {ParamType::INT, 0, 10, ParamDatatype::SINT8, 0, 0};
            ParamVariant variant{};
            variant._uint8Value = 16;
            formatter.formatParam(paramInfo, variant);
        }
        ASSERT_EQ("..", std::string(stream.getString()));
    }
    {
        PrintfFormatter formatter(stream, false);
        ParamInfo paramInfo = {ParamType::POS, 0, 0, ParamDatatype::SINT32PTR, 0, 0};
        ParamVariant variant{};
        int32_t pos             = 17;
        variant._sint32PtrValue = &pos;
        formatter.formatParam(paramInfo, variant);
        ASSERT_EQ(17, *variant._sint32PtrValue);
    }
}

} // anonymous namespace
