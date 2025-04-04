// Copyright 2024 Accenture.

#pragma once

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
