// Copyright 2024 Accenture.

#pragma once

#include "io/IReader.h"
#include "io/IWriter.h"

#include <estd/algorithm.h>
#include <estd/array.h>
#include <estd/big_endian.h>
#include <estd/memory.h>
#include <estd/slice.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace io
{

/**
 * Lock free single producer single consumer queue of variable size slices.
 * [TPARAMS_BEGIN]
 * \tparam CAPACITY Number of bytes that this MemoryQueue shall provide.
 * \tparam MAX_ELEMENT_SIZE Maximum size of one allocation
 * \tparam SIZE_TYPE Type used to store size of allocation internally
 * [TPARAMS_END]
 *
 * \section Memory overhead
 * In order to manage the internal storage, the MemoryQueue introduces an overhead of
 * sizeof(SIZE_TYPE) bytes per allocation.
 *
 * \section Alignment
 * The current implementation makes no assumptions about alignment. This is why the size of an
 * allocation is serialized using ::estd::write_be<>.
 *
 * \section Concurrency
 * This MemoryQueue is designed as a lock free single producer single consumer queue.
 *
 */
template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE = uint16_t>
class MemoryQueue
{
    static_assert(CAPACITY >= MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE), "");

    struct RxData
    {
        std::atomic<size_t> received{0U};
    };

    RxData rx;

    struct TxData
    {
        std::atomic<size_t> sent{0U};
        ::estd::array<uint8_t, CAPACITY> data;
        size_t allocated{0U};
        size_t minAvailable{CAPACITY};

        size_t available(RxData const& rxData) const;
    };

    TxData tx;

    static size_t advanceIndex(size_t index, size_t const size)
    {
        index = (index + size + sizeof(SIZE_TYPE)) % (2 * CAPACITY);

        // Check if enough contiguous space is available at the end of the array and wrap
        // otherwise skipping the too small piece of memory.
        size_t const space = (((2 * CAPACITY) - index) % CAPACITY);
        if (space < (MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE)))
        {
            index = (index + space) % (2 * CAPACITY);
        }
        return index;
    }

public:
    // [PUBLIC_TYPES_BEGIN]
    /** Type of the size information stored for each entry. */
    using size_type = SIZE_TYPE;

    // [PUBLIC_TYPES_END]

    // [PUBLIC_API_BEGIN]
    /**
     * Returns the capacity of the underlying array of data managed by this MemoryQueue.
     */
    static constexpr size_t capacity() { return CAPACITY; }

    /**
     * Returns the maximum size of one allocation.
     */
    static constexpr size_t maxElementSize() { return MAX_ELEMENT_SIZE; }

    /**
     * Constructs a MemoryQueue of CAPACITY bytes.
     */
    MemoryQueue() = default;

    // [PUBLIC_API_END]

    /**
     * The Writer side of a MemoryQueue provides API to insert data in the queue.
     */
    class Writer
    {
    public:
        // [PUBLIC_API_WRITER_BEGIN]
        /**
         * Constructs a Writer to a given queue.
         */
        explicit Writer(MemoryQueue& queue);

        /**
         * Allocates a requested number of bytes and returns them as a slice. This function can be
         * called multiple times before calling commit() allowing to first allocate a worst case
         * size slice and then trimming it before calling commit.
         *
         * \param size  Number of bytes to allocate from this MemoryQueue.
         * \return  - Empty slice, if requested size was greater as MAX_ELEMENT_SIZE or no memory
         *            is available
         *          - Slice of size bytes otherwise.
         */
        ::estd::slice<uint8_t> allocate(size_t size) const;

        /**
         * Makes the previously allocated data available for the Reader.
         */
        void commit();

        /**
         * Returns the number of contiguous bytes that can be allocated next.
         *
         * The calculation of used and free bytes relies on the fact that sent and received always
         * point to valid indices. That means, they will never point to an index where not at least
         * MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE) bytes are the difference to the end of the data
         * array.
         */
        size_t available() const;

        /**
         * Returns the minimum number of available bytes at the time of an allocate call since the
         * last reset using resetMinAvailable().
         */
        size_t minAvailable() const;

        /**
         * Resets the minimum number of available bytes to the current number of available bytes.
         */
        void resetMinAvailable();

        /**
         * The MemoryQueue is considered full if less than MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE)
         * bytes are available as a contiguous piece of memory.
         */
        bool full() const;

        /**
         * Returns the maximum size of which a slice of bytes can be allocated.
         */
        size_t maxSize() const;
        // [PUBLIC_API_WRITER_END]
    private:
        RxData const& _rxData;
        TxData& _txData;
    };

    /**
     * The Reader side of a MemoryQueue provides API to read data from the queue.
     */
    class Reader
    {
    public:
        // [PUBLIC_API_READER_BEGIN]
        /**
         * Constructs a Reader from a given queue.
         */
        explicit Reader(MemoryQueue& queue);

        /**
         * Returns true if no data is available.
         *
         * Calling peek() on an empty queue will return an empty slice.
         */
        bool empty() const;

        /**
         * Returns a slice of bytes pointing to the next memory chunk of the MemoryQueue, if
         * available. Calling peek on an empty MemoryQueue will return an empty slice.
         */
        ::estd::slice<uint8_t> peek() const;

        /**
         * Releases the first allocated chunk of memory. If the Reader is empty, calling this
         * function has no effect.
         */
        void release() const;

        /**
         * Releases all entries until empty() returns true.
         */
        void clear() const;

        /**
         * Returns the maximum size of which a slice of bytes can be read.
         */
        size_t maxSize() const;

        /**
         * Returns the number of contiguous bytes that can be allocated next.
         *
         * The calculation of used and free bytes relies on the fact that sent and received always
         * point to valid indices. That means, they will never point to an index where not at least
         * MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE) bytes are the difference to the end of the data
         * array.
         */
        size_t available() const;
        // [PUBLIC_API_READER_END]
    private:
        TxData& _txData;
        RxData& _rxData;
    };
};

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::Writer(MemoryQueue& queue)
: _rxData(queue.rx), _txData(queue.tx)
{}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
::estd::slice<uint8_t>
MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::allocate(size_t const size) const
{
    // Set allocated zero prevent a subsequent call to commit() to have
    // effects in case this allocation fails. That is important if this is a
    // reallocation, i.e. a previous call to allocate() succeeded.
    _txData.allocated = 0U;
    if ((size > MAX_ELEMENT_SIZE) || (size == 0))
    {
        return {};
    }
    auto const freeBytes = available();
    _txData.minAvailable = ::estd::min(freeBytes, _txData.minAvailable);
    if (freeBytes == 0)
    {
        return {};
    }
    size_t const index = _txData.sent.load() % CAPACITY;
    _txData.allocated  = size;
    return ::estd::memory::as_bytes(&_txData.data[index + sizeof(SIZE_TYPE)], size);
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
void MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::commit()
{
    // Prevent accidentally committing random data if allocate has not been called or
    // previous allocation was unsuccessful.
    if (_txData.allocated == 0U)
    {
        return;
    }
    size_t writeIndex    = _txData.sent.load();
    size_t const index   = writeIndex % CAPACITY;
    SIZE_TYPE const size = static_cast<SIZE_TYPE>(_txData.allocated);
    ::estd::write_be<SIZE_TYPE>(&_txData.data[index], static_cast<SIZE_TYPE>(size));

    writeIndex        = advanceIndex(writeIndex, size);
    _txData.allocated = 0U;
    // Store writeIndex last to ensure data consistency.
    _txData.sent.store(writeIndex);
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline size_t MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::available() const
{
    return _txData.available(_rxData);
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline size_t MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::minAvailable() const
{
    return _txData.minAvailable;
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline void MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::resetMinAvailable()
{
    _txData.minAvailable = available();
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline bool MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::full() const
{
    return available() == 0;
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline size_t MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Writer::maxSize() const
{
    return MAX_ELEMENT_SIZE;
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::Reader(MemoryQueue& queue)
: _txData(queue.tx), _rxData(queue.rx)
{}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline bool MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::empty() const
{
    size_t const readIndex  = _rxData.received.load();
    size_t const writeIndex = _txData.sent.load();
    return writeIndex == readIndex;
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline ::estd::slice<uint8_t>
MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::peek() const
{
    if (empty())
    {
        return {};
    }
    size_t const index   = _rxData.received.load() % CAPACITY;
    SIZE_TYPE const size = ::estd::read_be<SIZE_TYPE>(&_txData.data[index]);
    return ::estd::memory::as_bytes(&_txData.data[index + sizeof(SIZE_TYPE)], size);
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
void MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::release() const
{
    if (empty())
    {
        return;
    }
    size_t readIndex     = _rxData.received.load();
    size_t const index   = readIndex % CAPACITY;
    SIZE_TYPE const size = ::estd::read_be<SIZE_TYPE>(&_txData.data[index]);
    readIndex            = advanceIndex(readIndex, size);
    // Store readIndex last to ensure data consistency.
    _rxData.received.store(readIndex);
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline void MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::clear() const
{
    while (!empty())
    {
        release();
    }
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline size_t MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::maxSize() const
{
    return MAX_ELEMENT_SIZE;
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
inline size_t MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::Reader::available() const
{
    return _txData.available(_rxData);
}

template<size_t CAPACITY, size_t MAX_ELEMENT_SIZE, typename SIZE_TYPE>
size_t
MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>::TxData::available(RxData const& rxData) const
{
    size_t const writeIndex = sent.load();
    size_t const readIndex  = rxData.received.load();

    size_t usedBytes;
    if (writeIndex < readIndex)
    {
        usedBytes = (writeIndex + (2 * CAPACITY)) - readIndex;
    }
    else
    {
        usedBytes = writeIndex - readIndex;
    }

    size_t freeBytes = CAPACITY - usedBytes;
    if (freeBytes < (MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE)))
    {
        freeBytes = 0;
    }
    return freeBytes;
}

/**
 * Implementation of IWriter for MemoryQueue.
 * [TPARAMS_MQW_BEGIN]
 * \tparam Queue Type of queue to write data to.
 * [TPARAMS_MQW_END]
 */
template<class Queue>
class MemoryQueueWriter : public IWriter
{
public:
    // [PUBLIC_API_MQW_BEGIN]
    /**
     * Constructs a MemoryQueueWriter from given queue.
     */
    explicit MemoryQueueWriter(Queue& queue);

    /** \see IWriter::maxSize() */
    size_t maxSize() const override;

    /** \see IWriter::allocate() */
    ::estd::slice<uint8_t> allocate(size_t size) override;

    /** \see IWriter::commit() */
    void commit() override;

    /** \see IWriter::flush() */
    void flush() override;

    /** \see Queue::Writer::available() */
    size_t available() const;

    /** \see Queue::Writer::minAvailable() */
    size_t minAvailable() const;

    /** \see Queue::Writer::resetMinAvailable() */
    void resetMinAvailable();
    // [PUBLIC_API_MQW_END]
private:
    typename Queue::Writer _writer;
};

template<class Queue>
inline MemoryQueueWriter<Queue>::MemoryQueueWriter(Queue& queue) : _writer(queue)
{}

template<class Queue>
inline size_t MemoryQueueWriter<Queue>::maxSize() const
{
    return _writer.maxSize();
}

template<class Queue>
inline ::estd::slice<uint8_t> MemoryQueueWriter<Queue>::allocate(size_t const size)
{
    return _writer.allocate(size);
}

template<class Queue>
inline void MemoryQueueWriter<Queue>::commit()
{
    _writer.commit();
}

template<class Queue>
inline void MemoryQueueWriter<Queue>::flush()
{}

template<class Queue>
inline size_t MemoryQueueWriter<Queue>::available() const
{
    return _writer.available();
}

template<class Queue>
inline size_t MemoryQueueWriter<Queue>::minAvailable() const
{
    return _writer.minAvailable();
}

template<class Queue>
inline void MemoryQueueWriter<Queue>::resetMinAvailable()
{
    _writer.resetMinAvailable();
}

/**
 * Implementation of IReader for a MemoryQueue.
 * [TPARAMS_MQR_BEGIN]
 * \tparam Queue Type of queue to read data from.
 * [TPARAMS_MQR_END]
 */
template<class Queue>
class MemoryQueueReader : public IReader
{
public:
    // [PUBLIC_API_MQR_BEGIN]
    /**
     * Constructs a MemoryQueueReader from a given queue.
     */
    explicit MemoryQueueReader(Queue& queue);

    /** \see MemoryQueueReader::maxSize() */
    size_t maxSize() const override;

    /** \see MemoryQueueReader::peek() */
    ::estd::slice<uint8_t> peek() const override;

    /** \see MemoryQueueReader::release() */
    void release() override;

    /** \see Queue::Reader::available() */
    size_t available() const;
    // [PUBLIC_API_MQR_END]
private:
    typename Queue::Reader _reader;
};

template<class Queue>
inline MemoryQueueReader<Queue>::MemoryQueueReader(Queue& queue) : _reader(queue)
{}

template<class Queue>
inline size_t MemoryQueueReader<Queue>::maxSize() const
{
    return _reader.maxSize();
}

template<class Queue>
inline ::estd::slice<uint8_t> MemoryQueueReader<Queue>::peek() const
{
    return _reader.peek();
}

template<class Queue>
inline void MemoryQueueReader<Queue>::release()
{
    _reader.release();
}

template<class Queue>
inline size_t MemoryQueueReader<Queue>::available() const
{
    return _reader.available();
}

} // namespace io

