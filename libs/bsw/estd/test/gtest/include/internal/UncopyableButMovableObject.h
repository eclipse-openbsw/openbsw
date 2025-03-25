// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace internal
{
struct UncopyableButMovableObject
{
public:
    UncopyableButMovableObject() = default;
    explicit UncopyableButMovableObject(int32_t i) : i(i){};
    UncopyableButMovableObject(UncopyableButMovableObject const&)            = delete;
    UncopyableButMovableObject& operator=(UncopyableButMovableObject const&) = delete;

    // move assignment operator was implicitly deleted when the copy assignment operator was deleted
    UncopyableButMovableObject(UncopyableButMovableObject&&)            = default;
    UncopyableButMovableObject& operator=(UncopyableButMovableObject&&) = default;

    int32_t i{};
};

} /* namespace internal */

