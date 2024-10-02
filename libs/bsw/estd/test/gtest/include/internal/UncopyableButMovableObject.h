// Copyright 2024 Accenture.

#ifndef GUARD_4DFA7ED7_F0FD_44DC_A412_D38AD806C590
#define GUARD_4DFA7ED7_F0FD_44DC_A412_D38AD806C590

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

#endif /* GUARD_4DFA7ED7_F0FD_44DC_A412_D38AD806C590 */
