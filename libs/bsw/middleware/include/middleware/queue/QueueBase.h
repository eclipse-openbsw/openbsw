/********************************************************************************
 * Copyright (c) 2025, 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/atomic.h>
#include <etl/optional.h>

#include <cstdint>
#include <cstring>

namespace middleware::queue
{

/**
 * A struct that aggregates a series of statistics values related to queues.
 *
 */
struct QueueStats
{
    uint32_t processedMessages{0U};
    uint32_t lostMessages{0U};
    uint16_t loadSnapshot{0U};
    uint16_t processingCounter{0U};
    uint16_t realLoadSnapshot{0U};
    uint16_t realProcessingCounter{0U};
    uint8_t maxLoad{0U};
    uint8_t startupLoad{0U};
    uint8_t previousSnapshot{0U};
    uint8_t maxFillRate{0U};
};

/**
 * Base class for a multi-producer single-consumer queue, that is based on a circular buffer
 * implementation. This class contains a sent_ and received_ attributes which represent
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
    /** Returns a const reference to the queue statistics. */
    QueueStats const& getStats() const { return _stats; }

    /** Returns a reference to the queue statistics. */
    QueueStats& getStats() { return _stats; }

    /**
     * Resets the queues statistics.
     * \remark Must be protected with ECU mutex from caller, to ensure concistency.
     *
     */
    void resetStats() { _stats = QueueStats(); }

    /** Returns the current number of elements in the queue. */
    uint32_t size() const
    {
        // Snapshot both cursors once to avoid a race between the comparison
        // and the subtraction in the ternary expression below.
        // acquire on _sent synchronises with the release in publishSlot() so that
        // the payload written before publishSlot() is visible before peek().
        uint32_t const txPos = _sent.load(::etl::memory_order_acquire);
        uint32_t const rxPos = _received.load(::etl::memory_order_relaxed);
        return (txPos >= rxPos) ? (txPos - rxPos) : (txPos + (2U * _maxSize)) - rxPos;
    }

    /** Returns true if the queue is full, false otherwise. */
    bool isFull() const
    {
        uint32_t const rxPos = _received.load(::etl::memory_order_relaxed);
        return _sent.load(::etl::memory_order_relaxed) == ((rxPos + _maxSize) % (2U * _maxSize));
    }

    /** Returns true if the queue is empty, false otherwise. */
    bool isEmpty() const
    {
        // acquire on _sent: see size() comment.
        return _sent.load(::etl::memory_order_acquire)
               == _received.load(::etl::memory_order_relaxed);
    }

    /**
     * Update some of the statistic values of the queue, like the max fill rate and the
     * processingCounter. This method is useful to be called before processing the queue
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
    constexpr explicit QueueBase(uint32_t const maxSize)
    : _maxSize(maxSize), _sent(0U), _received(0U), _stats()
    {}

    /** Returns the value of the reading cursor. */
    uint32_t getReceived() const { return _received.load(::etl::memory_order_relaxed); }

    /** Returns the value of the writing cursor. */
    uint32_t getSent() const { return _sent.load(::etl::memory_order_relaxed); }

    /** Advances the reading cursor. */
    void advanceReceived()
    {
        uint32_t const next = (_received.load(::etl::memory_order_relaxed) + 1U) % (2U * _maxSize);
        _received.store(next, ::etl::memory_order_relaxed);
        ++_stats.processedMessages;
    }

    /**
     * Token returned by reserveSlot() carrying the slot index and the
     * next-sent value to pass to publishSlot() after writing the payload.
     */
    struct WriteToken
    {
        size_t slotIndex;
        uint32_t nextSent;
    };

    /**
     * Reserves the next writable slot without publishing the cursor.
     * The caller must write the payload into _buffer[token.slotIndex] and then
     * call publishSlot() to make the slot visible to the consumer.
     *
     * \return A WriteToken on success, or an empty optional if the queue is full.
     */
    ::etl::optional<WriteToken> reserveSlot()
    {
        ::etl::optional<WriteToken> token{};
        if (isFull())
        {
            ++_stats.lostMessages;
        }
        else
        {
            uint32_t const sentVal  = _sent.load(::etl::memory_order_relaxed);
            uint32_t const nextSent = (sentVal + 1U) % (2U * _maxSize);
            if (0U == _received.load(::etl::memory_order_relaxed))
            {
                ++_stats.startupLoad;
            }
            token.emplace(WriteToken{sentVal % _maxSize, nextSent});
        }
        return token;
    }

    /**
     * Publishes the slot reserved by reserveSlot(), making it visible to
     * the consumer. Must be called after the payload has been written.
     * Uses memory_order_release so the payload write is visible before
     * the consumer sees the updated cursor via size()/isEmpty()/peek().
     *
     * \param token  The WriteToken returned by the matching reserveSlot() call.
     */
    void publishSlot(WriteToken const& token)
    {
        _sent.store(token.nextSent, ::etl::memory_order_release);
        if (size() > _stats.maxLoad)
        {
            _stats.maxLoad = static_cast<uint8_t>(size());
        }
    }

private:
    uint32_t _maxSize;
    ::etl::atomic<uint32_t> _sent;
    ::etl::atomic<uint32_t> _received;
    QueueStats _stats;
};

} // namespace middleware::queue
