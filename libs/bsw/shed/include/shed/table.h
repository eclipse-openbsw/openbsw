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

#include "shed/table_internal.h" // IWYU pragma: export

#include <etl/utility.h>

#include <cstddef>
#include <cstdint>

namespace shed
{

template<typename Schema>
struct table
{
    using state_list  = typename Schema::states;
    using column_list = typename Schema::columns;

    using state_data  = ::shed::internal::state_data<state_list>;
    using column_data = ::shed::internal::column_data<state_data, column_list>;

    static constexpr size_t memory_for(size_t const num)
    {
        return internal::total_memory<column_data>::total(num) + internal::COLUMN_ALIGNMENT;
    }

    column_data columns;
    size_t n;

    bool init(::etl::span<uint8_t> mem, size_t const size)
    {
        if (mem.size() < memory_for(size))
        {
            return false;
        }
        if (!internal::multi_list::is_size_valid(size, state_list::size))
        {
            return false;
        }
        n = size;
        ft_for_each(columns, internal::init_column{n, mem});
        return true;
    }

    template<typename... Args>
    explicit table(Args&&... args) : columns(state_data(), ::etl::forward<Args>(args)...), n(0)
    {}

    table(table const&)            = delete;
    table& operator=(table const&) = delete;

    size_t size() const { return n; }
};

template<typename... StateTypes>
using states = ::shed::internal::type_list<internal::FREE, StateTypes...>;

template<typename... ColumnTypes>
using columns = ::shed::internal::type_list<ColumnTypes...>;

template<typename T>
struct shared
{
    using value_type = T;
    T value;

    template<typename... Args>
    explicit shared(Args&&... args) : value(::etl::forward<Args>(args)...)
    {}

    T& operator[](size_t const) { return value; }

    T const& operator[](size_t const) const { return value; }

    static void init(size_t const, ::etl::span<uint8_t>&) {}

    static constexpr size_t memory_for(size_t const) { return 0; }
};

template<typename T>
struct column
{
    static_assert(
        alignof(T) <= internal::COLUMN_ALIGNMENT, "columns types need to be word aligned");
    using value_type = T;
    ::etl::span<uint8_t> _data;

    ::etl::span<T const> data() const { return _data.reinterpret_as<T const>(); }

    ::etl::span<T> data() { return _data.reinterpret_as<T>(); }

    T& operator[](size_t const i) { return data()[i]; }

    T const& operator[](size_t const i) const { return data()[i]; }

    void init(size_t const n, ::etl::span<uint8_t>& s)
    {
        _data = s.first(n * sizeof(value_type));
        s.advance(n * sizeof(value_type));
    }

    static constexpr size_t memory_for(size_t const n) { return n * sizeof(value_type); }
};

template<typename... Cmp>
struct ordering
{
    using comparators = ::etl::type_list<Cmp...>;

    ::etl::span<internal::multi_list::idx_type> _scratch;

    void init(size_t const n, ::etl::span<uint8_t>& s)
    {
        _scratch = s.reinterpret_as<internal::multi_list::idx_type>().first(n);
        s.advance(n * sizeof(internal::multi_list::idx_type));
    }

    static constexpr size_t memory_for(size_t const n)
    {
        return n * sizeof(internal::multi_list::idx_type);
    }
};

} // namespace shed
