// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace internal
{
class UncopyableObject
{
public:
    UncopyableObject() = default;
    explicit UncopyableObject(int32_t i) : i(i){};
    UncopyableObject(UncopyableObject const&)            = delete;
    UncopyableObject& operator=(UncopyableObject const&) = delete;

    int32_t i{};
};

} /* namespace internal */
