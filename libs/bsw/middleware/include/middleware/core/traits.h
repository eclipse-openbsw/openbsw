// Copyright 2025 BMW AG

#pragma once

#include <etl/span.h>
#include <etl/type_traits.h>

#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
template<typename>
struct is_span : etl::false_type
{};

template<typename T, size_t N>
struct is_span<::etl::span<T, N>> : etl::true_type
{};

template<class T, class R = void>
struct enable_if_type
{
    using type = R;
};

template<class E, class Enable = void>
struct GetCopyPolicyType
{
    using type = void;
};

template<class E>
struct GetCopyPolicyType<E, typename enable_if_type<typename E::CopyPolicy>::type>
{
    using type = typename E::CopyPolicy;
};
} // namespace core
} // namespace middleware
