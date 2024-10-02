// Copyright 2024 Accenture.

#ifndef GUARD_8970498A_7257_483A_B0DD_25FDEE99963B
#define GUARD_8970498A_7257_483A_B0DD_25FDEE99963B

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

#endif /* GUARD_8970498A_7257_483A_B0DD_25FDEE99963B */
