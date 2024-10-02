// Copyright 2024 Accenture.

#include "util/format/PrintfFormatScanner.h"

#include <gtest/gtest.h>

using namespace ::util::format;

namespace
{
void checkEndToken(PrintfFormatScanner const& scanner)
{
    ASSERT_FALSE(scanner.hasToken());
    ASSERT_EQ(TokenType::END, scanner.getTokenType());
    ASSERT_EQ(0, *scanner.getTokenStart());
    ASSERT_EQ(scanner.getTokenEnd(), scanner.getTokenStart());
}

void checkStringToken(PrintfFormatScanner const& scanner, char const* pExpectedString)
{
    ASSERT_TRUE(scanner.hasToken());
    ASSERT_EQ(TokenType::STRING, scanner.getTokenType());
    ASSERT_EQ(0, strncmp(pExpectedString, scanner.getTokenStart(), strlen(pExpectedString)));
    ASSERT_EQ(scanner.getTokenEnd(), scanner.getTokenStart() + strlen(pExpectedString));
}

void checkParamToken(
    PrintfFormatScanner const& scanner,
    char const* pExpectedString,
    uint8_t flags,
    uint8_t base,
    int32_t width,
    int32_t precision,
    ParamType type,
    ParamDatatype datatype)
{
    ASSERT_TRUE(scanner.hasToken());
    ASSERT_EQ(TokenType::PARAM, scanner.getTokenType());
    ASSERT_EQ(0, strncmp(pExpectedString, scanner.getTokenStart(), strlen(pExpectedString)));
    ASSERT_EQ(scanner.getTokenEnd(), scanner.getTokenStart() + strlen(pExpectedString));
    ASSERT_EQ(flags, scanner.getParamInfo()._flags);
    ASSERT_EQ(base, scanner.getParamInfo()._base);
    ASSERT_EQ(width, scanner.getParamInfo()._width);
    ASSERT_EQ(precision, scanner.getParamInfo()._precision);
    ASSERT_EQ(type, scanner.getParamInfo()._type);
    ASSERT_EQ(datatype, scanner.getParamInfo()._datatype);
}

} // anonymous namespace

TEST(PrintfFormatScannerTest, testFormats)
{
    PrintfFormatScanner scanner("abcdefg"
                                "%- hc"
                                "between"
                                "%+#012s"
                                "%+#012S"
                                "%33.ln"
                                "%3.4QTest"
                                "%0*.*d"
                                "%0ld"
                                "%0lld"
                                "%0");
    checkStringToken(scanner, "abcdefg");
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%- hc",
        ParamFlags::FLAG_LEFT | ParamFlags::FLAG_SPACE,
        0,
        ParamWidthOrPrecision::DEFAULT,
        ParamWidthOrPrecision::DEFAULT,
        ParamType::CHAR,
        ParamDatatype::UINT8);
    scanner.nextToken();
    checkStringToken(scanner, "between");
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%+#012s",
        ParamFlags::FLAG_PLUS | ParamFlags::FLAG_ALT | ParamFlags::FLAG_ZEROPAD,
        0,
        12,
        ParamWidthOrPrecision::DEFAULT,
        ParamType::STRING,
        ParamDatatype::CHARPTR);
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%+#012S",
        ParamFlags::FLAG_PLUS | ParamFlags::FLAG_ALT | ParamFlags::FLAG_ZEROPAD,
        0,
        12,
        ParamWidthOrPrecision::DEFAULT,
        ParamType::STRING,
        ParamDatatype::SIZEDCHARPTR);
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%33.ln",
        0,
        0,
        33,
        ParamWidthOrPrecision::DEFAULT,
        ParamType::POS,
        ParamDatatype::SINT32PTR);
    scanner.nextToken();
    checkStringToken(scanner, "QTest");
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%0*.*d",
        ParamFlags::FLAG_ZEROPAD,
        10,
        ParamWidthOrPrecision::PARAM,
        ParamWidthOrPrecision::PARAM,
        ParamType::INT,
        ParamDatatype::SINT32);
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%0ld",
        ParamFlags::FLAG_ZEROPAD,
        10,
        ParamWidthOrPrecision::DEFAULT,
        ParamWidthOrPrecision::DEFAULT,
        ParamType::INT,
        ParamDatatype::SINT64);
    scanner.nextToken();
    checkParamToken(
        scanner,
        "%0lld",
        ParamFlags::FLAG_ZEROPAD,
        10,
        ParamWidthOrPrecision::DEFAULT,
        ParamWidthOrPrecision::DEFAULT,
        ParamType::INT,
        ParamDatatype::SINT64);
    scanner.nextToken();
    checkEndToken(scanner);
    scanner.nextToken();
    checkEndToken(scanner);
}

TEST(PrintfFormatScannerTest, testZeroFormat)
{
    PrintfFormatScanner scanner(nullptr);
    checkEndToken(scanner);
}
