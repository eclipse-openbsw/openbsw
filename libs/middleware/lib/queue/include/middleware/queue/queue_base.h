#pragma once

#include <etl/optional.h>

#include <cstdint>
#include <cstring>

namespace middleware::queue
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
    QueueStats const& getStats() const { return stats_; }

    /**
     * \brief Get a reference to the current queue statistics.
     *
     * \return QueueStats&
     */
    QueueStats& getStats() { return stats_; }

    /**
     * \brief Resets the queues statistics.
     * \remark Must be protected with ECU mutex from caller, to ensure concistency.
     *
     */
    void resetStats() { stats_ = QueueStats(); }

    /**
     * \brief Get the current size of the queue.
     *
     * \return constexpr uint32_t
     */
    uint32_t size() const
    {
        // Volatile is needed to to make sure the read is not optimized away when new elements have
        // been added to the queue by the other core.
        uint32_t const txPos = static_cast<uint32_t const volatile&>(sent_);
        return (txPos >= received_) ? (txPos - received_) : (txPos + (2U * maxSize_)) - received_;
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
            sent_
            == ((static_cast<uint32_t const volatile&>(received_) + maxSize_) % (2U * maxSize_)));
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
        return static_cast<uint32_t const volatile&>(sent_) == received_;
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
            stats_.loadSnapshot += static_cast<uint16_t>(currentSize);
            ++stats_.processingCounter;
        }
        stats_.realLoadSnapshot += static_cast<uint16_t>(currentSize);
        ++stats_.realProcessingCounter;
        if (stats_.previousSnapshot == 0U)
        {
            stats_.maxFillRate      = static_cast<uint8_t>(currentSize);
            stats_.previousSnapshot = static_cast<uint8_t>(currentSize);
        }
        else
        {
            if (currentSize > stats_.previousSnapshot)
            {
                auto const diff = (currentSize - stats_.previousSnapshot);
                if (diff > stats_.maxFillRate)
                {
                    stats_.maxFillRate = static_cast<uint8_t>(diff);
                }
            }
            stats_.previousSnapshot = static_cast<uint8_t>(currentSize);
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
        maxSize_  = maxSize;
        sent_     = 0U;
        received_ = 0U;
        stats_    = QueueStats();
    }

    /**
     * \brief Get the value of received_ attribute.
     *
     * \return uint32_t the value of the reading cursor.
     */
    uint32_t getReceived() const { return received_; }

    /**
     * \brief Get the value of sent_ attribute
     *
     * \return uint32_t the value of the writing cursor
     */
    uint32_t getSent() const { return sent_; }

    /**
     * \brief Advance the reading cursor.
     *
     */
    void advanceReceived()
    {
        received_ = (received_ + 1U) % (2U * maxSize_);
        ++stats_.processedMessages;
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
            ++stats_.lostMessages;
        }
        else
        {
            writableIndex.emplace(sent_ % maxSize_);
            sent_ = (sent_ + 1U) % (2U * maxSize_);
            if (size() > stats_.maxLoad)
            {
                stats_.maxLoad = static_cast<uint8_t>(size());
            }
            if (0U == stats_.processedMessages)
            {
                ++stats_.startupLoad;
            }
        }

        return writableIndex;
    }

private:
    uint32_t maxSize_;
    uint32_t sent_;
    uint32_t received_;
    QueueStats stats_;
};

} // namespace middleware::queue
