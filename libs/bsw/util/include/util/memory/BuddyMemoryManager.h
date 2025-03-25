// Copyright 2024 Accenture.

#pragma once

#include <estd/slice.h>
#include <estd/uncopyable.h>

#include <cstddef>
#include <cstdint>

namespace util
{
namespace memory
{

namespace internal
{
class BuddyMemoryManagerStatsHelper;
}

/**
 * Implementation of buddy memory allocation, which is configured through
 * its constructor parameters.
 *
 *
 * \section implementation Implementation
 * The internal implementation is based on a full binary tree. Initially
 * the root node will be marked free, and all other nodes will be link nodes.
 *
 * Suppose you created an instance of BuddyMemoryManager, with memorySize
 * 2052 and treeDepth 3. The initial binary tree would be
 * \dot
 * digraph binaryTree {
 *  graph [compound=true, center=true];
 *  node  [fontname="Helvetica", fontsize=10, shape=record];
 *
 *  node0 [label = "<f0> | <f1> Free (2052) | <f2>"];
 *  node1 [label = "<f0> | <f1> Link | <f2>"];
 *  node2 [label = "<f0> | <f1> Link | <f2>"];
 *  node3 [label = "<f0> | <f1> Link | <f2>"];
 *  node4 [label = "<f0> | <f1> Link | <f2>"];
 *  node5 [label = "<f0> | <f1> Link | <f2>"];
 *  node6 [label = "<f0> | <f1> Link | <f2>"];
 *
 *  "node0":f0 -> "node1":f1;
 *  "node0":f2 -> "node2":f1;
 *  "node1":f0 -> "node3":f1;
 *  "node1":f2 -> "node4":f1;
 *  "node2":f0 -> "node5":f1;
 *  "node2":f2 -> "node6":f1;
 * }
 * \enddot
 *
 * \subsection buddy The Buddy
 * The buddy of a node (this is where the name of the algorithm comes from) is
 * its sibling, i.e. its left neighbor, if the node is a right child or vice
 * versa.
 *
 * \subsection allocation Allocate Memory
 *
 * If you request a chunk of memory, the algorithm will look for the smallest
 * node that could handle your request. Note, that all nodes of a tree level
 * have the same size, so it'll actually determine the appropriate level and
 * look for a free node. If there's no free node, the algorithm will go up
 * one level, search a free node and split it. This is done until the root
 * node is reached.
 *
 * To continue with the example, let's suppose you requested 200 bytes of
 * memory, which would be in level 2. No node is free, as well as in level 1,
 * so the root node is split.
 * \dot
 * digraph binaryTree {
 *  graph [compound=true, center=true];
 *  node  [fontname="Helvetica", fontsize=10, shape=record];
 *
 *  node0 [label = "<f0> | <f1> Link | <f2>"];
 *  node1 [label = "<f0> | <f1> Free (1026) | <f2>"];
 *  node2 [label = "<f0> | <f1> Free (1026) | <f2>"];
 *  node3 [label = "<f0> | <f1> Link | <f2>"];
 *  node4 [label = "<f0> | <f1> Link | <f2>"];
 *  node5 [label = "<f0> | <f1> Link | <f2>"];
 *  node6 [label = "<f0> | <f1> Link | <f2>"];
 *
 *  "node0":f0 -> "node1":f1;
 *  "node0":f2 -> "node2":f1;
 *  "node1":f0 -> "node3":f1;
 *  "node1":f2 -> "node4":f1;
 *  "node2":f0 -> "node5":f1;
 *  "node2":f2 -> "node6":f1;
 * }
 * \enddot
 *
 * After that, the first node in level 1 will be split.
 * \dot
 * digraph binaryTree {
 *  graph [compound=true, center=true];
 *  node  [fontname="Helvetica", fontsize=10, shape=record];
 *
 *  node0 [label = "<f0> | <f1> Link | <f2>"];
 *  node1 [label = "<f0> | <f1> Link | <f2>"];
 *  node2 [label = "<f0> | <f1> Link | <f2>"];
 *  node3 [label = "<f0> | <f1> Free (513) | <f2>"];
 *  node4 [label = "<f0> | <f1> Free (513) | <f2>"];
 *  node5 [label = "<f0> | <f1> Link | <f2>"];
 *  node6 [label = "<f0> | <f1> Link | <f2>"];
 *
 *  "node0":f0 -> "node1":f1;
 *  "node0":f2 -> "node2":f1;
 *  "node1":f0 -> "node3":f1;
 *  "node1":f2 -> "node4":f1;
 *  "node2":f0 -> "node5":f1;
 *  "node2":f2 -> "node6":f1;
 * }
 * \enddot
 * Finally, the first node in level 2 will be marked used and the attached
 * chunk of memory will be returned.
 * \dot
 * digraph binaryTree {
 *  graph [compound=true, center=true];
 *  node  [fontname="Helvetica", fontsize=10, shape=record];
 *
 *  node0 [label = "<f0> | <f1> Link | <f2>"];
 *  node1 [label = "<f0> | <f1> Link | <f2>"];
 *  node2 [label = "<f0> | <f1> Link | <f2>"];
 *  node3 [label = "<f0> | <f1> Used (513) | <f2>"];
 *  node4 [label = "<f0> | <f1> Free (513) | <f2>"];
 *  node5 [label = "<f0> | <f1> Link | <f2>"];
 *  node6 [label = "<f0> | <f1> Link | <f2>"];
 *
 *  "node0":f0 -> "node1":f1;
 *  "node0":f2 -> "node2":f1;
 *  "node1":f0 -> "node3":f1;
 *  "node1":f2 -> "node4":f1;
 *  "node2":f0 -> "node5":f1;
 *  "node2":f2 -> "node6":f1;
 * }
 * \enddot
 *
 * \subsection free Free Memory
 * When you free a chunk of memory, the algorithm will locate the node and
 * see if it's buddy is also free. If so, it'll merge both and mark them link
 * nodes again and free the parent. This is done until the root node is reached.
 *
 * \subsection complexity Runtime Complexity
 * The runtime complexity of allocate and free is log2(n), where n is the
 * number of nodes in the tree, i.e. it is based on the depth of the tree.
 */
class BuddyMemoryManager
{
    UNCOPYABLE(BuddyMemoryManager);

public:
    static constexpr uint8_t TAG_NODE_FREE = 0x02U;
    static constexpr uint8_t TAG_NODE_LINK = 0x01U;
    static constexpr uint8_t TAG_NODE_USED = 0x00U;

    class AcquireResult
    {
    public:
        AcquireResult(size_t const index, size_t const num) : _bucketIndex(index), _numBuckets(num)
        {}

        AcquireResult() : AcquireResult(0U, 0U) {}

        size_t firstBucketIndex() const { return _bucketIndex; }

        size_t numBuckets() const { return _numBuckets; }

        bool isValid() const { return _numBuckets != 0U; }

    private:
        size_t _bucketIndex;
        size_t _numBuckets;
    };

    /**
     * Constructs an instance of BuddyMemoryManager.
     * \param nodeTree Slice of internal binary tree.
     */
    BuddyMemoryManager(::estd::slice<uint8_t> nodeTree);

    size_t numBuckets() const;

    /**
     * Marks the complete memory free.
     */
    void clear();

    bool isEmpty() const;

    /**
     * Acquires a given number of buckets.
     * \return
     * - Bucket index and number of buckets acquired
     * - Invalid result if the requested amount of buckets was not available
     */
    AcquireResult acquireMemory(size_t numBuckets);

    /**
     * Frees a given chunk of memory.
     * \return
     * - true if memory starting with bucket at bucketIndex was occupied
     * - false otherwise
     */
    bool releaseMemory(size_t bucketIndex);

    /**
     * Frees a given chunk of memory.
     * \return Number of freed buckets. Zero on error;
     */
    size_t releaseMemoryExtended(size_t bucketIndex);

private:
    friend ::util::memory::internal::BuddyMemoryManagerStatsHelper;

    template<typename T>
    static T fastLog2(T value);

    template<typename T>
    static T clzToLog2(T value);

    bool isNodeFree(size_t nodePosition) const;

    bool isNodeUsed(size_t nodePosition) const;

    static size_t parentPosition(size_t nodePosition);

    static bool isLeftSibling(size_t nodePosition);

    void tagNodeUsed(size_t nodePosition);

    void tagNodeFree(size_t nodePosition);

    void tagNodeLink(size_t nodePosition);

    void splitNodeUntilLevelReached(size_t nodePosition, size_t level, size_t targetLevel);

    AcquireResult findBucketInLevel(size_t level);

    void freeNode(size_t nodePosition);

    static size_t buddy(size_t nodePosition);

    void tryToSplitParentLevel(size_t level);

    size_t levelToBucketNum(size_t level) const;

    /**
     * Erases internal node tree and sets all nodes to free.
     */
    void clearNodeTree();

    uint8_t* const _nodeTree;
    size_t const _nodeTreeDepth;
    size_t const _numBuckets;
    size_t const _nodeTreeSize;
};

inline size_t BuddyMemoryManager::numBuckets() const { return _numBuckets; }

namespace internal
{
template<size_t N>
struct TwoToThePowerOf
{
    /** result */
    static size_t const VALUE = static_cast<size_t>(1U) << N;
};

template<size_t N>
struct Log2
{
    static size_t const VALUE = Log2<N / 2U>::VALUE + 1U;
};

template<>
struct Log2<1U>
{
    static size_t const VALUE = 0U;
};

// Will not compile
template<>
struct Log2<0U>
{};

template<size_t N>
struct RoundToNextPowerOf2
{
    static size_t const VALUE = TwoToThePowerOf<Log2<N * 2U - 1U>::VALUE>::VALUE;
};

class BuddyMemoryManagerStatsHelper
{
public:
    static uint8_t* getNodeTree(BuddyMemoryManager const& manager) { return manager._nodeTree; }

    static size_t getNodeTreeDepth(BuddyMemoryManager const& manager)
    {
        return manager._nodeTreeDepth;
    }
};
} // namespace internal

namespace declare
{
/**
 * A configurable implementation of buddy memory allocation.
 *
 * \tparam MIN_NUM_BUCKETS Minimum number of buckets.
 *
 * \see BuddyMemoryManager
 *
 * \section MIN_NUM_BUCKETS MIN_NUM_BUCKETS
 * This number determines the minimum number of buckets the user needs to manage. It will be
 * adjusted to the next power of 2.
 */
template<size_t MIN_NUM_BUCKETS>
class BuddyMemoryManager : public ::util::memory::BuddyMemoryManager
{
    UNCOPYABLE(BuddyMemoryManager);

public:
    /** Number of buckets */
    static size_t const NUM_BUCKETS
        = ::util::memory::internal::RoundToNextPowerOf2<MIN_NUM_BUCKETS>::VALUE;

    BuddyMemoryManager();

private:
    /** Size of the internal binary node tree */
    static size_t const NODE_TREE_ARRAY_SIZE = NUM_BUCKETS * 2U - 1U;

    // node tree array is first for cache locality
    uint8_t _nodeTreeArray[NODE_TREE_ARRAY_SIZE] = {};
};

template<size_t MIN_NUM_BUCKETS>
BuddyMemoryManager<MIN_NUM_BUCKETS>::BuddyMemoryManager()
: ::util::memory::BuddyMemoryManager(::estd::make_slice(_nodeTreeArray))
{
    clear();
}
} // namespace declare
} // namespace memory
} // namespace util

