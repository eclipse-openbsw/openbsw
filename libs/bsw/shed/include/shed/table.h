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

#include <etl/error_handler.h>

#include <cstddef>
#include <cstdint>

namespace shed
{

template<typename Schema, size_t stack_depth = 1U>
struct table
{
    using state_list  = typename Schema::states;
    using column_list = typename Schema::columns;

    using state_data  = ::shed::internal::state_data<state_list>;
    using column_data = ::shed::internal::column_data<state_data, column_list>;

    static constexpr size_t memory_for(size_t const num)
    {
        return internal::total_memory<column_data>::total(num)
               + num * stack_depth * sizeof(internal::multi_list::idx_type)
               + stack_depth * sizeof(::etl::span<internal::multi_list::idx_type>)
               + internal::COLUMN_ALIGNMENT;
    }

    ::etl::span<::etl::span<internal::multi_list::idx_type>> _scratch_arrays;
    size_t pos;
    column_data columns;
    size_t n;

    bool init(::etl::span<uint8_t> mem, size_t const size)
    {
        if (mem.size() < memory_for(size))
        {
            return false;
        }
        n = size;
        _scratch_arrays
            = mem.reinterpret_as<::etl::span<internal::multi_list::idx_type>>().first(stack_depth);
        mem.advance(stack_depth * sizeof(::etl::span<internal::multi_list::idx_type>));
        for (auto& scratch_array : _scratch_arrays)
        {
            scratch_array = mem.reinterpret_as<internal::multi_list::idx_type>().first(n);
            mem.advance(n * sizeof(internal::multi_list::idx_type));
        }
        ft_for_each(columns, internal::init_column{n, mem});
        return true;
    }

    struct Scratch
    {
        Scratch(
            ::etl::span<::etl::span<internal::multi_list::idx_type>> const scratch_arrays,
            size_t& pos)
        : _pos(pos)
        {
            ETL_ASSERT(
                pos < scratch_arrays.size(), ETL_ERROR_GENERIC("shed: scratch space exhausted"));
            _scratch = scratch_arrays[_pos];
            ++_pos;
        }

        ~Scratch() { --_pos; }

        ::etl::span<internal::multi_list::idx_type> _scratch;
        size_t& _pos;
    };

    Scratch alloc_scratch() { return Scratch(_scratch_arrays, pos); }

    template<typename... Args>
    explicit table(Args&&... args)
    : _scratch_arrays(), pos(0), columns(state_data(), std::forward<Args>(args)...), n(0)
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
    explicit shared(Args&&... args) : value(std::forward<Args>(args)...)
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

} // namespace shed
