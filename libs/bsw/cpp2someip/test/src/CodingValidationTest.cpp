/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/CodingValidation.h"

#include <gtest/gtest.h>

namespace
{
class CodingValidator
{
public:
    static bool _isValid;

    static bool validate(float_t /* f */) { return _isValid; }
};

// static
bool CodingValidator::_isValid;

class ComplexType
{
public:
    bool _isValid;

    bool isValid() const { return _isValid; }
};

TEST(CodingValidation, validation_of_simple_coding_success)
{
    ::etl::array<float_t, 2U> a{};
    a[0] = 1.0;
    a[1] = 3.0;

    CodingValidator c;
    c._isValid = true;

    EXPECT_TRUE(::someip::types::validateSimpleCoding<CodingValidator>(a));
}

TEST(CodingValidation, validation_of_simple_coding_failure)
{
    ::etl::array<float, 2U> a{};
    a[0] = 1.0;
    a[1] = 3.0;

    CodingValidator c;
    c._isValid = false;

    EXPECT_FALSE(::someip::types::validateSimpleCoding<CodingValidator>(a));
}

TEST(CodingValidation, validation_of_complex_coding_success)
{
    ::etl::array<ComplexType, 2U> a{};
    a[0]._isValid = true;
    a[1]._isValid = true;

    EXPECT_TRUE(::someip::types::validateComplexCoding(a));
}

TEST(CodingValidation, validation_of_complex_coding_failure)
{
    ::etl::array<ComplexType, 2U> a{};
    a[0]._isValid = true;
    a[1]._isValid = false;

    EXPECT_FALSE(::someip::types::validateComplexCoding(a));
}

} // anonymous namespace
