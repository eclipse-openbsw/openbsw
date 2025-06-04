// Copyright 2025 Accenture.

#pragma once

#include <estd/slice.h>

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

        ::estd::slice<T> const* operator->() const { return &_current->getBuffer(); }

        ::estd::slice<T> const& operator*() const { return _current->getBuffer(); }

        ::estd::slice<T>* operator->() { return &_current->getBuffer(); }

        ::estd::slice<T>& operator*() { return _current->getBuffer(); }

        LinkedBuffer<T>* getCurrent() const { return _current; }

    private:
        friend class LinkedBuffer;

        Iterator(LinkedBuffer<T>& first) : _current(&first) {}

        LinkedBuffer<T>* _current;
    };

    explicit LinkedBuffer() : _next(nullptr) {}

    explicit LinkedBuffer(::estd::slice<T> buffer, LinkedBuffer* next = nullptr)
    : _buffer(buffer), _next(next)
    {}

    LinkedBuffer(LinkedBuffer const&)            = delete;
    LinkedBuffer& operator=(LinkedBuffer const&) = delete;

    ::estd::slice<T>& getBuffer() { return _buffer; }

    void setBuffer(::estd::slice<T> buffer) { _buffer = buffer; }

    LinkedBuffer<T>* getNext() const { return _next; }

    void setNext(LinkedBuffer<T>* next) { _next = next; }

    Iterator begin() { return Iterator(*this); }

    Iterator end() { return Iterator(); }

private:
    ::estd::slice<T> _buffer;
    LinkedBuffer<T>* _next;
};

} // namespace util
