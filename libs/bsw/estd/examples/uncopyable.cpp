// Copyright 2024 Accenture.

#include "estd/uncopyable.h"

#include <gtest/gtest.h>

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-private-field"

/*Below code shows how to make a class uncopyable that cannot inherit
  from the uncopyable base class.
 */
// [EXAMPLE_BEGIN uncopyable macro]
class MyClass
{
    UNCOPYABLE(MyClass);

public:
    MyClass(int n) : n(n) {}

private:
    int n;
};

void example_uncopyable_macro()
{
    MyClass obj(10);
    // Usage of copy construction calls will result in compilation error.
    // MyClass obj1 = obj;
    // MyClass obj2(obj);
}

// [EXAMPLE_END uncopyable macro]
// Below code shows how to make a class uncopyable by inheriting from uncopyable
// [EXAMPLE_BEGIN uncopyable class]
class MyClass1 : public ::estd::uncopyable
{
public:
    MyClass1(int n) : n(n) {}

private:
    int n;
};

void example_uncopyable_inheritance()
{
    MyClass1 obj(10);
    // Usage of copy construction calls will result in compilation error.
    // MyClass1 obj1 = obj;
    // MyClass1 obj2(obj);
}

// [EXAMPLE_END uncopyable class]

TEST(uncopyable, run_examples)
{
    example_uncopyable_inheritance();
    example_uncopyable_macro();
}
