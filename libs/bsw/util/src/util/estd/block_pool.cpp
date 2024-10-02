// Copyright 2024 Accenture.

#include "util/estd/block_pool.h"

#include <estd/assert.h>

namespace util
{
namespace estd
{
block_pool::block_pool(uint8_t* const buffer, size_type const bufferSize, size_type const blockSize)
: _buffer(buffer)

, _blockSize((blockSize >= sizeof(free_block)) ? blockSize : sizeof(free_block))
, _blockCount(bufferSize / _blockSize)
, _usedBlockCount(0U)
, _firstFreeBlock()
{
    estd_assert(_blockSize % sizeof(void*) == 0);
    clear();
}

bool block_pool::contains(uint8_t const* const object) const
{
    return (object >= _buffer) && (object < (_buffer + _blockCount * _blockSize));
}

size_t block_pool::index_of(uint8_t const* const object) const
{
    return static_cast<size_t>(object - _buffer) / _blockSize;
}

uint8_t* block_pool::acquire()
{
    estd_assert(_firstFreeBlock != nullptr);
    uint8_t* const block = reinterpret_cast<uint8_t*>(_firstFreeBlock);
    _firstFreeBlock      = _firstFreeBlock->_next;
    ++_usedBlockCount;
    return block;
}

void block_pool::release(uint8_t* const object)
{
    estd_assert(contains(object));
    releaseImpl(object);
    --_usedBlockCount;
}

void block_pool::clear()
{
    _firstFreeBlock            = nullptr;
    uint8_t* const bufferStart = _buffer;
    for (size_type idx = 0U; idx < _blockCount; ++idx)
    {
        releaseImpl(bufferStart + (_blockSize * idx));
    }
    _usedBlockCount = 0;
}

} // namespace estd
} // namespace util
