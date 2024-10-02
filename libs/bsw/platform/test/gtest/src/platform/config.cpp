// Copyright 2024 Accenture.

#include "platform/config.h"

namespace
{
ESR_PACKED_BEGIN

struct A
{
    int a : 2;
    int b : 6;
};

ESR_PACKED_END

struct B
{
    int a : 2;
    int b : 6;
};

static_assert(sizeof(A) == 1, "ESR_PACKED_BEGIN should work");
static_assert(sizeof(B) != 1, "ESR_PACKED_END should work");

} // namespace
