// Copyright 2024 Accenture.

#include "estd/bitset.h"

#include <gtest/gtest.h>

namespace
{
void example_construct()
{
    // EXAMPLE_START construct
    ::estd::bitset<16> a(0x0000);
    a[0] = 1;               // Bitset now equals 0x0001
    ::estd::bitset<1000> b; // Bitsets can have arbitrarily large size
    // EXAMPLE_END construct
}

void example_operators()
{
    // EXAMPLE_START operators
    ::estd::bitset<16> a(0x00FF);
    ::estd::bitset<16> b(0x0F0F);
    a = ~a;                             // a now equals 0xFF00
    a &= b;                             // a now equals 0x0F00
    a = b | ::estd::bitset<16>(0xFF00); // a now equals 0xFF0F
    // EXAMPLE_END operators
}

void example_element_access()
{
    // EXAMPLE_START element_access
    ::estd::bitset<16> a(0x0000);
    a[0] = true;  // a now equals 0x0001
    a[4].flip();  // a now equals 0x0011
    a[0] = ~a[0]; // a now equals 0x0010
    // EXAMPLE_END element_access
}

void example_operations()
{
    // EXAMPLE_START operations
    ::estd::bitset<16> a; // a equals 0x0000
    a.any();              // False
    a.all();              // False
    a.none();             // True

    a.set(4); // a equals 0x0010
    a.any();  // True
    a.all();  // False
    a.none(); // False

    a.value(); // returns 2
    a.flip();  // a equals 0xFFEF
    a.test(0); // True

    a.set();  // a equals 0xFFFF
    a.any();  // True
    a.all();  // True
    a.none(); // False

    a.reset(); // a equals 0x0000
    a.size();  // returns 16
    // EXAMPLE_END operations
}

} // namespace

TEST(bitset_test, run_examples)
{
    example_construct();
    example_operators();
    example_element_access();
    example_operations();
}
