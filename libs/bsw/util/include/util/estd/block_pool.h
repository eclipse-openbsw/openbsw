// Copyright 2024 Accenture.

#pragma once

#include <estd/algorithm.h>
#include <estd/type_list.h>

#include <cstddef>
#include <cstdint>

namespace util
{

namespace estd
{
class block_pool
{
public:
    /** An unsigned integral type for sizes */
    using size_type = std::size_t;

    /**
     * Constructor to initialize this block_pool with the actual data to use.
     * Used by the declare class.
     *
     * \param buffer The buffer to use for storing the entries
     * \param bufferSize size of the buffer
     * \param blockSize the minimum size for a block.
     *
     * \see ::estd::declare::block_pool
     */
    block_pool(uint8_t buffer[], size_type bufferSize, size_type blockSize);

    block_pool(block_pool const&)            = delete;
    block_pool& operator=(block_pool const&) = delete;

    /**
     * Returns whether this object pool can be used to acquire additional objects.
     */
    bool empty() const { return _firstFreeBlock == nullptr; }

    /**
     * Returns true if none of this block_pool's objects have been acquired.
     */
    bool full() const { return _usedBlockCount == 0U; }

    /**
     * Returns the size of a block from this block_pool.
     */
    size_type block_size() const { return _blockSize; }

    /**
     * Returns the number of objects that can be acquired from this block_pool.
     */
    size_type size() const
    {
        return static_cast<size_type>(_blockCount) - static_cast<size_type>(_usedBlockCount);
    }

    /**
     * Returns the maximum number of objects this block_pool can hold.
     */
    size_type max_size() const { return _blockCount; }

    /**
     * Returns whether a given object belongs to this block_pool's internal memory.
     */
    bool contains(uint8_t const* object) const;

    /**
     * Returns the unique block index of an object.
     * \assert{contains(object)}
     * \param object object to get index for
     */
    size_t index_of(uint8_t const* object) const;

    /**
     * Acquires a block of memory from this block_pool and returns a reference to it
     * decreasing this block_pool's size by one.
     * \assert{!empty()}
     */
    uint8_t* acquire();

    /**
     * Releases an acquired block of memory to this pool increasing this block_pool's
     * size by one.
     * It is checked if the object being released belongs to this block_pool.
     */
    void release(uint8_t* object);

    /**
     * Simply clears all blocks and makes them free for use again. Any pointers to blocks
     * are no more valid after this call.
     */
    void clear();

private:
    struct free_block
    {
        free_block* _next;
    };

    /**
     * Helper method for release method
     */
    inline void releaseImpl(uint8_t* const object)
    {
        free_block* const freeBlock = reinterpret_cast<free_block*>(object);
        freeBlock->_next            = _firstFreeBlock;
        _firstFreeBlock             = freeBlock;
    }

    uint8_t* _buffer;
    size_type _blockSize;
    size_type _blockCount;
    size_type _usedBlockCount;
    free_block* _firstFreeBlock;
};

namespace declare
{
/**
 * The actual implementation of the block_pool with a defined number of
 * entries of defined size.
 * \tparam  N   Number of blocks this object_pool manages.
 * \tparam  B   Size of a single block
 */
template<std::size_t N, std::size_t B>
class block_pool : public ::util::estd::block_pool
{
public:
    static_assert(
        B % sizeof(void*) == 0,
        "Blocksize B needs to be a multiple of the platform's pointer size for alignment.");

    /**
     * Constructor to initialize the block bool with N blocks of size B.
     */
    block_pool() : ::util::estd::block_pool(&_buffer[0], sizeof(_buffer), BlockSize) {}

private:
    using block_type_list = ::estd::type_list<uint8_t[B], ::estd::type_list<uint8_t*>>;

    static std::size_t const BlockSize
        = ::estd::maximum<std::size_t, B, ::estd::max_size<block_type_list>::value>::value;

    uint8_t _buffer[N * BlockSize];
};
} // namespace declare

} // namespace estd
} // namespace util

