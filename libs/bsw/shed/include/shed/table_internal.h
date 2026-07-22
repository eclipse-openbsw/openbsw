/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "shed/id.h"
#include "shed/multi_list.h"

#include <etl/span.h>
#include <etl/type_list.h>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace shed
{
template<typename T>
struct shared;

template<typename T>
struct column;

namespace internal
{
static constexpr size_t COLUMN_ALIGNMENT = alignof(::std::max_align_t);

template<typename... Types>
struct type_list
{
    using TL_type                = ::etl::type_list<Types...>;
    static constexpr size_t size = sizeof...(Types);
};

template<typename... Types>
struct flat_tuple : Types...
{
    template<typename... Args>
    explicit flat_tuple(Args&&... args) : Args(std::forward<Args>(args))...
    {}

    flat_tuple(flat_tuple const&) = default;
};

template<typename T>
struct total_memory
{
    static constexpr size_t total(size_t const n) { return T::memory_for(n); }
};

template<>
struct total_memory<::shed::internal::flat_tuple<>>
{
    static constexpr size_t total(size_t const) { return 0; }
};

template<typename T, typename... Types>
struct total_memory<::shed::internal::flat_tuple<T, Types...>>
{
    static constexpr size_t total(size_t const n)
    {
        return T::memory_for(n) + (COLUMN_ALIGNMENT - T::memory_for(n) % COLUMN_ALIGNMENT)
               + total_memory<::shed::internal::flat_tuple<Types...>>::total(n);
    }
};

template<typename... Types>
void do_nothing(Types const...)
{}

template<typename F, typename T>
void* ignore_return(F&& f, T&& t)
{
    std::forward<F>(f)(std::forward<T>(t));
    return nullptr;
}

template<typename F, typename... Types>
void ft_for_each(flat_tuple<Types...>& args, F&& f)
{
    do_nothing(ignore_return(std::forward<F>(f), *static_cast<Types*>(&args))...);
}

template<typename F, typename... Types>
void ft_for_each(flat_tuple<Types...> const& args, F&& f)
{
    do_nothing(ignore_return(std::forward<F>(f), *static_cast<Types*>(&args))...);
}

struct FREE
{};

struct init_column
{
    init_column() = default;

    init_column(size_t const size, ::etl::span<uint8_t> const& memory) : n(size), mem(memory) {}

    size_t n = 0;
    ::etl::span<uint8_t> mem;

    template<typename T>
    void operator()(T& v)
    {
        auto const addr = reinterpret_cast<uintptr_t>(mem.data());
        auto const pad  = (COLUMN_ALIGNMENT - (addr % COLUMN_ALIGNMENT)) % COLUMN_ALIGNMENT;
        mem.advance(pad);
        v.init(n, mem);
    }
};

template<typename types>
struct state_data
{
    using value_type = id;

    id operator[](size_t const i) const
    {
        return id(static_cast<decltype(id::idx)>(i), generations[i]);
    }

    static constexpr size_t memory_for(size_t const n)
    {
        return multi_list::memory_for(n, types::size) + n * sizeof(decltype(id::generation));
    }

    void init(size_t const n, ::etl::span<uint8_t>& ml_mem)
    {
        ml          = multi_list::make(n, types::size, ml_mem);
        generations = ml_mem.reinterpret_as<decltype(id::generation)>().first(n);
        ml_mem.advance(n * sizeof(decltype(id::generation)));
    }

    multi_list* ml = nullptr;
    ::etl::span<decltype(id::generation)> generations;
    decltype(id::generation) generation = 1;
};

template<typename StateData, typename>
struct column_data_type;

template<typename StateData, typename... Types>
struct column_data_type<StateData, ::shed::internal::type_list<Types...>>
{
    using type = flat_tuple<StateData, Types...>;
};

template<typename StateData, typename ColumnList>
using column_data = typename column_data_type<StateData, ColumnList>::type;

} // namespace internal
} // namespace shed
