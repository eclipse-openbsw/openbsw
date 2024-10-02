// Copyright 2024 Accenture.

#ifndef GUARD_C2B820D3_FF64_42D7_A2E3_ACE2B88C83A1
#define GUARD_C2B820D3_FF64_42D7_A2E3_ACE2B88C83A1

#include <platform/estdint.h>

namespace internal
{
struct NoDefaultConstructorObject
{
    NoDefaultConstructorObject() = delete;
    explicit NoDefaultConstructorObject(int32_t i) : i(i){};

    int32_t i;
};

} /* namespace internal */

#endif /* GUARD_C2B820D3_FF64_42D7_A2E3_ACE2B88C83A1 */
