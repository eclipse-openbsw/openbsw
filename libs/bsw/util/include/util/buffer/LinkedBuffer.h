/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/span.h>

namespace util
{
namespace buffer
{

/*
 * A helper class for keeping buffers (i.e. ETL spans of type T) in a singly linked list.
 */
template<typename T>
class LinkedBuffer
{
public:
    class Iterator
    {
    public:
        Iterator& operator++()
        {
            if (_currentBuf != nullptr)
            {
                _currentBuf = _currentBuf->getNext();
            }
            return *this;
        }

        bool operator==(Iterator const& other) const { return _currentBuf == other._currentBuf; }

        bool operator!=(Iterator const& other) const { return _currentBuf != other._currentBuf; }

        ::etl::span<T> const* operator->() const { return &_currentBuf->getBuffer(); }

        ::etl::span<T> const& operator*() const { return _currentBuf->getBuffer(); }

        ::etl::span<T>* operator->() { return &_currentBuf->getBuffer(); }

        ::etl::span<T>& operator*() { return _currentBuf->getBuffer(); }

    private:
        friend class LinkedBuffer;

        explicit Iterator() : _currentBuf(nullptr) {}

        explicit Iterator(LinkedBuffer<T>& first) : _currentBuf(&first) {}

        LinkedBuffer<T>* _currentBuf;
    };

    explicit LinkedBuffer() : _next(nullptr) {}

    explicit LinkedBuffer(::etl::span<T> const buffer, LinkedBuffer* const next = nullptr)
    : _buffer(buffer), _next(next)
    {}

    LinkedBuffer(LinkedBuffer const&)            = delete;
    LinkedBuffer& operator=(LinkedBuffer const&) = delete;

    ::etl::span<T>& getBuffer() { return _buffer; }

    void setBuffer(::etl::span<T> const buffer) { _buffer = buffer; }

    LinkedBuffer<T>* getNext() const { return _next; }

    void setNext(LinkedBuffer<T>* const next) { _next = next; }

    Iterator begin() { return Iterator(*this); }

    Iterator end() { return Iterator(); }

private:
    ::etl::span<T> _buffer;
    LinkedBuffer<T>* _next;
};

} // namespace buffer
} // namespace util
