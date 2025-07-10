// Copyright 2025 Accenture.

#pragma once

#include <etl/span.h>

namespace util
{

// to be placed in util?
template<typename T>
class LinkedBuffer
{
public:
    class Iterator
    {
    public:
        Iterator() : _current(nullptr) {}

        Iterator& operator++()
        {
            _current = _current->getNext();
            return *this;
        }

        bool operator==(Iterator const& other) const { return _current == other._current; }

        bool operator!=(Iterator const& other) const { return _current != other._current; }

        ::etl::span<T> const* operator->() const { return &_current->getBuffer(); }

        ::etl::span<T> const& operator*() const { return _current->getBuffer(); }

        ::etl::span<T>* operator->() { return &_current->getBuffer(); }

        ::etl::span<T>& operator*() { return _current->getBuffer(); }

        LinkedBuffer<T>* getCurrent() const { return _current; }

    private:
        friend class LinkedBuffer;

        Iterator(LinkedBuffer<T>& first) : _current(&first) {}

        LinkedBuffer<T>* _current;
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

} // namespace util
