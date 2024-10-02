// Copyright 2024 Accenture.

#include "estd/type_utils.h"

#include <gtest/gtest.h>

// clang-format off
namespace
{
void example_cast_to_type()
{
    // [EXAMPLE_BEGIN cast to type]
    int16_t data[4] = {55, 55 , 35, 25};
    uint8_t* raw_data = reinterpret_cast<uint8_t*>(data);
    estd::type_utils<int>::cast_to_type(raw_data, 2);
    // [EXAMPLE_END cast to type]
}
void example_const_cast_to_type()
{
    // [EXAMPLE_BEGIN cast const to type]
    int16_t data[4] = {55, 55 , 35, 25};
    uint8_t* raw_data = reinterpret_cast<uint8_t*>(data);
    estd::type_utils<uint16_t>::cast_const_to_type(raw_data, 2);
    // *result = 12;     //Cannot modify the value, since its a const data
    // [EXAMPLE_END cast const to type]
}

void example_cast_to_raw()
{
    // [EXAMPLE_BEGIN cast to raw]
    int value = 42;
    int* int_ptr = &value;
    estd::type_utils<int>::cast_to_raw(int_ptr);
    // Now, you can manipulate raw_ptr to read or modify the underlying bytes.
    // [EXAMPLE_END cast to raw]
}

void example_cast_from_void()
{
    // [EXAMPLE_BEGIN cast from void]
    int value = 42;
    void* void_ptr = &value;
    estd::type_utils<int>::cast_from_void(void_ptr);
    // Now, you can use int_ptr as a pointer to an integer.
    // [EXAMPLE_END cast from void]
}

void example_const_cast_from_void()
{
    // [EXAMPLE_BEGIN const cast from void]
    int data = 10;
    void* vPtr = &data;
    estd::type_utils<int>::const_cast_from_void(vPtr);
    //  *cPtr = 20;     //Cannot modify the value, since its a const data
    // Now, you have a const int pointer.
    // [EXAMPLE_END const cast from void]
}

void example_cast_to_void()
{
    // [EXAMPLE_BEGIN cast to void]
    int value = 42;
    int* int_ptr = &value;
    estd::type_utils<int>::cast_to_void(int_ptr);
    // Now, void_ptr can be used to store or pass the integer pointer.
    // [EXAMPLE_END cast to void]
}

void example_const_cast_to_void()
{
    // [EXAMPLE_BEGIN const cast to void]
    const int value = 42;
    const int* int_ptr = &value;
    estd::type_utils<int>::const_cast_to_void(int_ptr);
    // Now, void_ptr can be used to store or pass the integer pointer.
    // [EXAMPLE_END const cast to void]
}
} // namespace

TEST( type_utils, run_examples)
{
     example_cast_to_type();
     example_const_cast_to_type();
     example_cast_to_void();
     example_const_cast_from_void();
     example_cast_from_void();
     example_cast_to_raw();
     example_const_cast_to_void();
}