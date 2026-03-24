// Copyright 2025 BMW AG

#pragma once

#include "middleware/memory/PoolBase.h"

#include <etl/array.h>
#include <etl/span.h>

#include <climits>
#include <cstdint>

namespace middleware
{
namespace memory
{

template<size_t N, size_t ChunkSize>
class Pool : public PoolBase
{
    static size_t const STORAGE_FLAGS_SIZE
        = static_cast<size_t>(N % static_cast<size_t>(CHAR_BIT)) == 0U
              ? static_cast<size_t>(N / static_cast<size_t>(CHAR_BIT))
              : static_cast<size_t>((N / static_cast<size_t>(CHAR_BIT)) + 1U);

    struct ElementType
    {
        etl::array<uint8_t, ChunkSize> chunk;
    };

    // Ensure alignment is suitable for storing pointers in freed chunks.
    struct alignas(alignof(uint8_t*)) AlignedElementType : ElementType
    {};

    etl::array<AlignedElementType, N> _storage;
    etl::array<uint8_t, STORAGE_FLAGS_SIZE> _storageFlags;

    static uint8_t* getStorage(etl::array<AlignedElementType, N>& storage)
    {
        return storage.data()->chunk.data();
    }

    static uint8_t* getFlags(etl::array<uint8_t, STORAGE_FLAGS_SIZE>& flags)
    {
        return flags.data();
    }

public:
    using base_t     = PoolBase;
    using value_type = AlignedElementType;
    using pointer    = AlignedElementType*;
    using size_type  = size_t;

    Pool(Pool const&)            = delete;
    Pool(Pool&&)                 = delete;
    Pool& operator=(Pool const&) = delete;
    Pool& operator=(Pool&&)      = delete;

    // Constructor intentionally leaves non-constant members uninitialised.
    // Shared-RAM pools are constructed asynchronously by all cores.
    Pool()
    : base_t(getStorage(_storage), chunkSize(), valueSize(), capacity(), getFlags(_storageFlags))
    {}

    /** Returns the fixed chunk size for this pool. */
    static constexpr size_type chunkSize() { return ChunkSize; }

    /** Returns the size of the stored element type. */
    static constexpr size_type valueSize() { return sizeof(AlignedElementType); }

    /** Returns the fixed capacity (number of elements) for this pool. */
    static constexpr size_type capacity() { return N; }

    /** Equality operator */
    friend bool operator==(Pool const& lhs, Pool const& rhs) { return &lhs == &rhs; }
};

} // namespace memory
} // namespace middleware
