// Copyright 2025 BMW AG

#include "middleware/memory/PoolBase.h"

#include <etl/algorithm.h>
#include <etl/iterator.h>
#include <etl/memory.h>
#include <etl/tuple.h>

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace middleware::memory
{

PoolBase::PoolBase(
    uint8_t* const buff,
    size_t const elementSize,
    size_t const elementAlignedSize,
    size_t const elementCount,
    uint8_t* const flagsBuff)
: _buffer(buff)
, _elementSize(elementSize)
, _elementAlignedSize(elementAlignedSize)
, _elementCount(elementCount)
, _flags(flagsBuff)
{}

void PoolBase::initialize()
{
    resetStats();
    etl::mem_set(_buffer, _elementAlignedSize * _elementCount, static_cast<uint8_t>(0));
    size_t const flagsSize = ((_elementCount % static_cast<size_t>(CHAR_BIT)) == 0U)
                                 ? (_elementCount / static_cast<size_t>(CHAR_BIT))
                                 : (_elementCount / static_cast<size_t>(CHAR_BIT)) + 1U;
    etl::mem_set(_flags, flagsSize, static_cast<uint8_t>(0));

    uint8_t* tempBuff = _buffer;
    for (size_t i = 0U; i < (_elementCount - 1U); ++i)
    {
        uint8_t* const tempNext                = offsetAddress(tempBuff, _elementAlignedSize);
        // Write the address of tempNext in the memory pointed to by tempBuff
        *reinterpret_cast<uint8_t**>(tempBuff) = tempNext; // NOLINT
        tempBuff                               = tempNext;
    }
    // Last chunk: next pointer is nullptr
    *reinterpret_cast<uint8_t**>(tempBuff) = nullptr; // NOLINT
    _nextChunk                             = _buffer;
    _available                             = _elementCount;
}

uint8_t* PoolBase::allocate(size_t const size)
{
    uint8_t* ret = nullptr;

    if ((size <= _elementSize) && (!isFull()))
    {
        ret        = _nextChunk;
        _nextChunk = *reinterpret_cast<uint8_t**>(_nextChunk); // NOLINT
        --_available;
        _stats.internalFragmentation += static_cast<uint32_t>(_elementSize - size);
        ++_stats.successfulAllocations;
        _stats.maxLoad = etl::max(
            _stats.maxLoad, static_cast<decltype(PoolStats::maxLoad)>(_elementCount - _available));

        ptrdiff_t const distance = etl::distance(_buffer, ret);
        size_t const ptrPosition = static_cast<size_t>(distance) / _elementAlignedSize;
        updatePtrFlag(ptrPosition, true);
    }

    return ret;
}

bool PoolBase::deallocate(void* const ptr)
{
    bool ret              = false;
    auto* const ptrObject = static_cast<uint8_t*>(ptr);
    if (isValidPointer(ptrObject))
    {
        *reinterpret_cast<uintptr_t*>(ptrObject)                                    // NOLINT
            = _nextChunk != nullptr ? reinterpret_cast<uintptr_t>(_nextChunk) : 0U; // NOLINT

        ptrdiff_t const distance = etl::distance(_buffer, ptrObject);
        size_t const ptrPosition = static_cast<size_t>(distance) / _elementAlignedSize;
        updatePtrFlag(ptrPosition, false);
        ++_available;
        _nextChunk = ptrObject;
        ret        = true;
    }

    return ret;
}

bool PoolBase::isEmpty() const { return _available == _elementCount; }

bool PoolBase::isFull() const { return _available == 0U; }

size_t PoolBase::available() const { return _available; }

size_t PoolBase::size() const { return _elementCount - _available; }

size_t PoolBase::maxSize() const { return _elementCount; }

uint8_t* PoolBase::offsetAddress(uint8_t* ptr, size_t const offset)
{
    etl::advance(ptr, offset);
    return ptr;
}

bool PoolBase::isValidPointer(uint8_t const* const ptr) const
{
    bool res = false;

    auto const ptrValue = reinterpret_cast<uintptr_t>(ptr);     // NOLINT
    auto const start    = reinterpret_cast<uintptr_t>(_buffer); // NOLINT
    auto const end      = reinterpret_cast<uintptr_t>(          // NOLINT
        offsetAddress(_buffer, _elementCount * _elementAlignedSize));

    bool const isInRange  = (ptrValue >= start) && (ptrValue <= (end - _elementAlignedSize));
    bool const isMultiple = ((ptrValue - start) % _elementAlignedSize) == 0U;
    if (isInRange && isMultiple)
    {
        ptrdiff_t const distance = etl::distance(static_cast<uint8_t const*>(_buffer), ptr);
        size_t const ptrPosition = static_cast<size_t>(distance) / _elementAlignedSize;
        res                      = checkPtrFlag(ptrPosition);
    }

    return res;
}

void PoolBase::updatePtrFlag(size_t const position, bool const ptrBusy)
{
    size_t const index          = position / static_cast<size_t>(CHAR_BIT);
    size_t const indexRemainder = position % static_cast<size_t>(CHAR_BIT);
    if (ptrBusy)
    {
        *(offsetAddress(_flags, index)) |= static_cast<uint8_t>(1U << indexRemainder);
    }
    else
    {
        uint32_t const mask = ~(static_cast<size_t>(1U) << indexRemainder);
        *(offsetAddress(_flags, index)) &= static_cast<uint8_t>(mask);
    }
}

bool PoolBase::checkPtrFlag(size_t const position) const
{
    size_t const index          = position / static_cast<size_t>(CHAR_BIT);
    size_t const indexRemainder = position % static_cast<size_t>(CHAR_BIT);
    auto const bitFlag          = static_cast<uint8_t>(1U << indexRemainder);
    return static_cast<bool>(*(offsetAddress(_flags, index)) & bitFlag);
}

PoolStats& PoolBase::getPoolStats() { return _stats; }

void PoolBase::resetStats()
{
    _stats.init();
    _stats.chunkSize = static_cast<uint32_t>(_elementSize);
    _stats.capacity  = static_cast<uint32_t>(_elementCount);
}

etl::tuple<size_t, size_t, double> PoolBase::getProfile() const
{
    return etl::make_tuple(
        _available,
        _stats.maxLoad,
        ((_stats.successfulAllocations > 0U)
             ? static_cast<double>(_stats.internalFragmentation)
                   / static_cast<double>(_stats.successfulAllocations)
             : 0.0));
}

} // namespace middleware::memory
