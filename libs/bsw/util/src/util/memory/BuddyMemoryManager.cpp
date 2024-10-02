// Copyright 2024 Accenture.

#include "util/memory/BuddyMemoryManager.h"

#include <estd/memory.h>

#include <cstring>

namespace util
{
namespace memory
{
template<typename T>
T BuddyMemoryManager::fastLog2(T value)
{
    T result = 0U;

    while (value > 1U)
    {
        value >>= 1U;
        ++result;
    }
    return result;
}

template<typename T>
inline T BuddyMemoryManager::clzToLog2(T const value)
{
    return static_cast<T>(sizeof(T)) * static_cast<T>(8U) - static_cast<T>(1U) - value;
}

#if defined(__GNUC__)

using ClzType   = unsigned int;
using ClzlType  = unsigned long;
using ClzllType = unsigned long long;

template<>
inline ClzType BuddyMemoryManager::fastLog2<>(ClzType const value)
{
    return clzToLog2(static_cast<ClzType>(__builtin_clz(value)));
}

template<>
inline ClzlType BuddyMemoryManager::fastLog2<>(ClzlType const value)
{
    return clzToLog2(static_cast<ClzlType>(__builtin_clzl(value)));
}

template<>
inline ClzllType BuddyMemoryManager::fastLog2<>(ClzllType const value)
{
    return clzToLog2(static_cast<ClzllType>(__builtin_clzll(value)));
}

#endif

BuddyMemoryManager::BuddyMemoryManager(::estd::slice<uint8_t> const nodeTree)
: _nodeTree(nodeTree.data())
, _nodeTreeDepth(fastLog2<size_t>(nodeTree.size() + 1U))
, _numBuckets(static_cast<size_t>(1U) << (_nodeTreeDepth - 1U))
, _nodeTreeSize(nodeTree.size())
{
    clear();
}

void BuddyMemoryManager::clear() { clearNodeTree(); }

inline void BuddyMemoryManager::tagNodeUsed(size_t const nodePosition)
{
    _nodeTree[nodePosition] = TAG_NODE_USED;
}

inline void BuddyMemoryManager::tagNodeFree(size_t const nodePosition)
{
    _nodeTree[nodePosition] = TAG_NODE_FREE;
}

inline void BuddyMemoryManager::tagNodeLink(size_t const nodePosition)
{
    _nodeTree[nodePosition] = TAG_NODE_LINK;
}

inline bool BuddyMemoryManager::isLeftSibling(size_t const nodePosition)
{
    return (nodePosition & 1U) != 0U;
}

inline bool BuddyMemoryManager::isNodeFree(size_t const nodePosition) const
{
    return (_nodeTree[nodePosition] == TAG_NODE_FREE);
}

inline bool BuddyMemoryManager::isNodeUsed(size_t const nodePosition) const
{
    return (_nodeTree[nodePosition] == TAG_NODE_USED);
}

inline size_t BuddyMemoryManager::parentPosition(size_t const nodePosition)
{
    return (nodePosition - 1U) >> 1U;
}

inline size_t BuddyMemoryManager::levelToBucketNum(size_t const level) const
{
    return static_cast<size_t>(1U) << (_nodeTreeDepth - 1U - level);
}

BuddyMemoryManager::AcquireResult BuddyMemoryManager::acquireMemory(size_t numBuckets)
{
    if (numBuckets > _numBuckets)
    {
        return AcquireResult();
    }
    if (numBuckets == 0U)
    {
        numBuckets = 1U;
    }

    size_t const level         = _nodeTreeDepth - fastLog2<size_t>((numBuckets << 1U) - 1U) - 1U;
    AcquireResult const bucket = findBucketInLevel(level);
    if (bucket.isValid())
    {
        return bucket;
    }
    if (level == 0U)
    {
        return AcquireResult();
    }
    tryToSplitParentLevel(level);
    return findBucketInLevel(level);
}

BuddyMemoryManager::AcquireResult BuddyMemoryManager::findBucketInLevel(size_t const level)
{
    size_t const numberOfNodesInLevel = static_cast<size_t>(1U) << level;
    size_t const startPositionOfLevel = numberOfNodesInLevel - 1U;
    for (size_t i = startPositionOfLevel; i < (startPositionOfLevel + numberOfNodesInLevel); ++i)
    {
        if (isNodeFree(i))
        {
            size_t const bucketNum = levelToBucketNum(level);
            size_t const index     = (i - startPositionOfLevel) * bucketNum;
            tagNodeUsed(i);
            return AcquireResult(index, bucketNum);
        }
    }
    return AcquireResult();
}

bool BuddyMemoryManager::releaseMemory(size_t const bucketIndex)
{
    return releaseMemoryExtended(bucketIndex) != 0U;
}

size_t BuddyMemoryManager::releaseMemoryExtended(size_t const bucketIndex)
{
    size_t numBuckets           = _numBuckets;
    size_t numberOfNodesInLevel = 1U;
    for (size_t level = 0U; level < _nodeTreeDepth; ++level)
    {
        // replace (bucketIndex % numBuckets) with bit operations
        // numBuckets is power of 2, so instead of using a very costly operator% we can
        // resort to bit arithmetic, as for power of 2 numbers only 1 bit is set
        if ((bucketIndex & (numBuckets - 1U)) == 0U)
        {
            size_t const startPositionOfLevel = numberOfNodesInLevel - 1U;
            // replace (bucketIndex / numBuckets) with bit operators
            // numBuckets is power of 2, so we can just bit shift it by log2(numBuckets)
            size_t const i = startPositionOfLevel + (bucketIndex >> (_nodeTreeDepth - level - 1U));
            if (i >= (startPositionOfLevel + numberOfNodesInLevel))
            {
                return 0U;
            }
            if (isNodeUsed(i))
            {
                freeNode(i);
                return numBuckets;
            }
        }
        numBuckets >>= 1U;
        numberOfNodesInLevel <<= 1U;
    }
    return 0U;
}

void BuddyMemoryManager::freeNode(size_t nodePosition)
{
    while (nodePosition > 0U)
    {
        size_t const buddyPosition = buddy(nodePosition);
        if (isNodeFree(buddyPosition))
        {
            tagNodeLink(nodePosition);
            tagNodeLink(buddyPosition);
            nodePosition = parentPosition(nodePosition);
        }
        else
        {
            tagNodeFree(nodePosition);
            return;
        }
    }
    tagNodeFree(nodePosition);
}

inline size_t BuddyMemoryManager::buddy(size_t const nodePosition)
{
    if (isLeftSibling(nodePosition))
    {
        return nodePosition + 1U;
    }

    return nodePosition - 1U;
}

void BuddyMemoryManager::tryToSplitParentLevel(size_t const level)
{
    size_t currentLevel = level - 1U;
    while (true)
    {
        size_t const numberOfNodesInLevel = static_cast<size_t>(1U) << currentLevel;
        size_t const startPositionOfLevel = numberOfNodesInLevel - 1U;
        for (size_t i = startPositionOfLevel; i < (startPositionOfLevel + numberOfNodesInLevel);
             ++i)
        {
            if (isNodeFree(i))
            {
                splitNodeUntilLevelReached(i, currentLevel, level);
                return;
            }
        }
        if (currentLevel == 0U)
        {
            return;
        }
        --currentLevel;
    }
}

void BuddyMemoryManager::splitNodeUntilLevelReached(
    size_t nodePosition, size_t level, size_t const targetLevel)
{
    do
    {
        tagNodeLink(nodePosition);
        size_t const leftChild  = (nodePosition << 1U) + 1U;
        size_t const rightChild = leftChild + 1U;
        tagNodeFree(leftChild);
        tagNodeFree(rightChild);

        ++level;
        nodePosition = leftChild;
    } while (level < targetLevel);
}

void BuddyMemoryManager::clearNodeTree()
{
    ::estd::memory::set(
        ::estd::slice<uint8_t>::from_pointer(_nodeTree, _nodeTreeSize), TAG_NODE_LINK);

    _nodeTree[0U] = TAG_NODE_FREE;
}

bool BuddyMemoryManager::isEmpty() const
{
    if (_nodeTree[0U] != TAG_NODE_FREE)
    {
        return false;
    }
    for (size_t i = 1U; i < _nodeTreeSize; ++i)
    {
        if (_nodeTree[i] != TAG_NODE_LINK)
        {
            return false;
        }
    }
    return true;
}
} // namespace memory
} // namespace util
