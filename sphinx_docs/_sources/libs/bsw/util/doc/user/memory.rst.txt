.. _util_memory:

`util::memory`
==============

Some utilities that allow working with memory, including lower-level bit operations and customer memory management.

Bit Operations
--------------

The set of template functions providing reversing bits in the integer.

.. code-block:: cpp

    TypeParam value    = static_cast<TypeParam>(0xF0F0F0F0F0F0F0F0ULL);
    TypeParam expected = static_cast<TypeParam>(0x0F0F0F0F0F0F0F0FULL);
    assert(memory::reverseBits(value) == expected);

util::memory::BuddyMemoryManager
--------------------------------

The class implements a memory management algorithm to efficiently allocate memory blocks required by the client.
The available memory pool is divided in halves until a suitable memory block size is identified.
When a client releases a block, the algorithm attempts to "fuse" the freed block
with its sibling if the sibling is also free. If possible, the freeing process is
propagated upward in the hierarchy. This approach allows fast searching for blocks
of appropriate size and reduces memory fragmentation.

.. code-block:: cpp

    static size_t const NUM_BUCKETS = 8U;

    using MyBuddyMemoryManager =
        ::util::memory::declare::BuddyMemoryManager<NUM_BUCKETS>;
    using Bucket = MyBuddyMemoryManager::AcquireResult;

    MyBuddyMemoryManager _memoryManager;
    // returns Bucket(0U, 8U), bucket with index 0 and size 8 bytes:
    auto bucket = _memoryManager.acquireMemory(8U);
    // returns false:
    _memoryManager.isEmpty();
    // expected to release 8 bytes from first block:
    size_t freed = _memoryManager.releaseMemoryExtended(0U);

