// Copyright 2025 BMW AG

#pragma once

#include <etl/optional.h>

#include <cstdint>
#include <cstring>

namespace middleware
{
namespace queue
{

/**
 * \brief A struct that aggregates a series of statistics values related to queues.
 *
 */
struct QueueStats
{
    uint32_t processedMessages;
    uint32_t lostMessages;
    uint16_t loadSnapshot;
    uint16_t processingCounter;
    uint16_t realLoadSnapshot;
    uint16_t realProcessingCounter;
    uint8_t maxLoad;
    uint8_t startupLoad;
    uint8_t previousSnapshot;
    uint8_t maxFillRate;
};

/**
 * \brief Base class for a multi-producer single-consumer queue, that is based on a circular buffer
 * implementation. \details This class contains a sent_ and received_ attributes which represent
 * writing and reading cursors, which will always be in the range [0, 2*maxSize[, where maxSize is
 * an attribute that is initialized by the init method, that represents the queue's maximum number
 * of elements. This way there are two distinct constellations where sent_ and received_ point to
 * the same element: 1) sent_ == received_ 2) sent_ == (received_ + MAX_SIZE) % (2*MAX_SIZE). Using
 * this trick the ambiguity between the empty and full cases of the queue is resolved without the
 * need for an unused element in the queue: Case 1) means "empty" and case 2) means "full".
 *
 */
class QueueBase
{
public:
    /**
     * \brief Get a constant reference to the current queue statistics.
     *
     * \return const QueueStats&
     */
    QueueStats const& getStats() const { return _stats; }

    /**
     * \brief Get a reference to the current queue statistics.
     *
     * \return QueueStats&
     */
    QueueStats& getStats() { return _stats; }

    /**
     * \brief Resets the queues statistics.
     * \remark Must be protected with ECU mutex from caller, to ensure concistency.
     *
     */
    void resetStats() { _stats = QueueStats(); }

    /**
     * \brief Get the current size of the queue.
     *
     * \return constexpr uint32_t
     */
    uint32_t size() const
    {
        // Volatile is needed to to make sure the read is not optimized away when new elements have
        // been added to the queue by the other core.
        uint32_t const txPos = static_cast<uint32_t const volatile&>(_sent);
        return (txPos >= _received) ? (txPos - _received) : (txPos + (2U * _maxSize)) - _received;
    }

    /**
     * \brief Check if the queue is full.
     *
     * \return true if full, otherwise false.
     */
    bool isFull() const
    {
        // Volatile is needed to make sure the read is not optimized away when full() is called in a
        // loop like `while(sender.full()){}`.
        return (
            _sent
            == ((static_cast<uint32_t const volatile&>(_received) + _maxSize) % (2U * _maxSize)));
    }

    /**
     * \brief Check if the queue is empty.
     *
     * \return true if empty, otherwise false.
     */
    bool isEmpty() const
    {
        // Volatile is needed to prevent `isEmpty()` from returning `true` when in fact new elements
        // have been added to the queue by the other core.
        return static_cast<uint32_t const volatile&>(_sent) == _received;
    }

    /**
     * \brief Update some of the statistic values of the queue, like the max fill rate and the
     * processingCounter. \details This method is useful to be called before processing the queue
     * elements (reading and advancing). \remark A mutex is not needed here, since this will only be
     * called by a unique consumer.
     *
     */
    void takeSnapshot()
    {
        uint32_t const currentSize = size();
        if (0U != currentSize)
        {
            _stats.loadSnapshot += static_cast<uint16_t>(currentSize);
            ++_stats.processingCounter;
        }
        _stats.realLoadSnapshot += static_cast<uint16_t>(currentSize);
        ++_stats.realProcessingCounter;
        if (_stats.previousSnapshot == 0U)
        {
            _stats.maxFillRate      = static_cast<uint8_t>(currentSize);
            _stats.previousSnapshot = static_cast<uint8_t>(currentSize);
        }
        else
        {
            if (currentSize > _stats.previousSnapshot)
            {
                auto const diff = (currentSize - _stats.previousSnapshot);
                if (diff > _stats.maxFillRate)
                {
                    _stats.maxFillRate = static_cast<uint8_t>(diff);
                }
            }
            _stats.previousSnapshot = static_cast<uint8_t>(currentSize);
        }
    }

protected:
    QueueBase() {}

    /**
     * \brief Init method which needs to be called before doing any work with the queue.
     *
     *
     * \param maxSize the maximum number of elements inside the queue.
     */
    void init(uint32_t const maxSize)
    {
        _maxSize                     = maxSize;
        _sent                        = 0U;
        _received                    = 0U;
        _stats.processedMessages     = 0U;
        _stats.lostMessages          = 0U;
        _stats.loadSnapshot          = 0U;
        _stats.processingCounter     = 0U;
        _stats.realLoadSnapshot      = 0U;
        _stats.realProcessingCounter = 0U;
        _stats.maxLoad               = 0U;
        _stats.startupLoad           = 0U;
        _stats.previousSnapshot      = 0U;
        _stats.maxFillRate           = 0U;
    }

    /**
     * \brief Get the value of received_ attribute.
     *
     * \return uint32_t the value of the reading cursor.
     */
    uint32_t getReceived() const { return _received; }

    /**
     * \brief Get the value of sent_ attribute
     *
     * \return uint32_t the value of the writing cursor
     */
    uint32_t getSent() const { return _sent; }

    /**
     * \brief Advance the reading cursor.
     *
     */
    void advanceReceived()
    {
        _received = (_received + 1U) % (2U * _maxSize);
        ++_stats.processedMessages;
    }

    /**
     * \brief Updates the writing cursor to the next element in the buffer.
     *
     * \return etl::optional<size_t>
     */
    etl::optional<size_t> writeNext()
    {
        etl::optional<size_t> writableIndex{};
        if (isFull())
        {
            ++_stats.lostMessages;
        }
        else
        {
            writableIndex.emplace(_sent % _maxSize);
            _sent = (_sent + 1U) % (2U * _maxSize);
            if (size() > _stats.maxLoad)
            {
                _stats.maxLoad = static_cast<uint8_t>(size());
            }
            if (0U == _stats.processedMessages)
            {
                ++_stats.startupLoad;
            }
        }

        return writableIndex;
    }

private:
    uint32_t _maxSize;
    uint32_t _sent;
    uint32_t _received;
    QueueStats _stats;
};

} // namespace queue
} // namespace middleware
