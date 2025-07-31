// Copyright 2024 Accenture.

#pragma once

#include <etl/array.h>

#include <platform/estdint.h>

#include <atomic>

namespace util
{
namespace spsc
{
namespace internal
{
template<class T, size_t N, bool TrackConsumed>
struct TxData;
}

/**
 * Lock-free implementation of a single-producer, single-consumer queue.
 * Provides two nested classes "Sender" and "Receiver" to be used on the sending and receiving
 * end.
 * Both get passed a reference to the queue at construction time.
 *
 * Optionally a third template parameter can be provided.
 * This parameter is a boolean (defaulting to false) that, if true, creates a Queue that
 * will keep track of what elements have been consumed by the receiving end.
 * In this case the method Queue::Sender::checkConsumed(F) can be used with F being a callable
 * that will be called for each element that is already consumed.
 */
template<
    class T,
    uint16_t N, // N is intentionally a smaller type than size_t used for storing
                // the indices, to ensure (2*N) does never overflow.
    bool TrackConsumed = false>
class Queue
{
    // Just make sure, in case someone ever compiles this for a 16bit or 8bit target.
    static_assert(sizeof(uint16_t) < sizeof(size_t), "");

    // The tx.sent and rx.received variables are always in the range [0, 2*N] while the ring
    // contains N elements. This way there are two distinct constellations where
    // tx.sent and rx.received point two the same element:
    // 1) tx.sent == rx.received
    // 2) tx.sent == rx.received + N
    // Using this trick the ambiguity between the empty and full cases of the queue is resolved
    // without the need for an unused element in the queue: Case 1) means "empty" and case 2) means
    // "full".
    //
    using TxData = internal::TxData<T, N, TrackConsumed>;

    TxData tx;

    struct RxData
    {
        std::atomic<size_t> received;

        RxData() : received(0U) {}
    };

    RxData rx;

public:
    using value_type = T;

    Queue() = default;

    void reset()
    {
        rx.received.store(0U);
        tx.reset();
    }

    size_t capacity() const { return N; }

    class Receiver
    {
    public:
        class Read
        {
            Read(Read const&) = delete;

            Receiver& _r;

        public:
            explicit Read(Receiver& r) : _r(r) {}

            T* operator->() const { return &_r.peek(); }

            T& operator*() const { return _r.peek(); }

            ~Read() { _r.advance(); }
        };

        explicit Receiver(Queue& queue) : _txData(queue.tx), _rxData(queue.rx) {}

        bool empty() const
        {
            size_t const received = _rxData.received.load();
            size_t const sent     = _txData.sent.load();
            return sent == received;
        }

        T read() { return *Read(*this); }

        void advance()
        {
            size_t const received = _rxData.received.load();
            _rxData.received.store((received + 1U) % (2U * static_cast<size_t>(N)));
        }

        T& peek() const { return _txData.data[_rxData.received.load() % N]; }

        size_t size() const
        {
            size_t const received = _rxData.received.load();
            size_t const sent     = _txData.sent.load();
            return (sent + ((sent < received) ? 1U : 0U) * (2U * N)) - received;
        }

        void clear()
        {
            while (!empty())
            {
                (void)advance();
            }
        }

    private:
        TxData& _txData;
        RxData& _rxData;
    };

    class Sender
    {
    public:
        class Write
        {
            Write(Write const&) = delete;

            Sender& _s;

        public:
            explicit Write(Sender& s) : _s(s) {}

            T* operator->() const { return &_s.next(); }

            T& operator*() const { return _s.next(); }

            ~Write() { _s.write_next(); }
        };

        explicit Sender(Queue& queue) : _rxData(queue.rx), _txData(queue.tx) {}

        void write(T const& value) { *Write(*this) = value; }

        template<class F>
        void checkConsumed(F const& onConsumed)
        {
            _txData.check(_rxData, onConsumed);
        }

        T& next() const { return _txData.data[_txData.sent.load() % N]; }

        void write_next()
        {
            size_t const sent = _txData.sent.load();
            _txData.sent.store((sent + 1U) % (2U * static_cast<size_t>(N)));
        }

        size_t size() const
        {
            size_t const sent     = _txData.sent.load();
            size_t const received = _rxData.received.load();
            return (sent + ((sent < received) ? 1U : 0U) * (2U * N)) - received;
        }

        bool full() const { return _txData.full(_rxData); }

    private:
        RxData const& _rxData;
        TxData& _txData;
    };
};

namespace internal
{
template<class T, size_t N, bool>
struct TxData
{
    std::atomic<size_t> sent;
    std::atomic<size_t> acked;
    ::etl::array<T, N> data;

    TxData() : sent(0U), acked(0U), data() {}

    void reset()
    {
        sent.store(0U);
        acked.store(0U);
    }

    template<class RxData, class OnConsumed>
    void check(RxData const& rxData, OnConsumed const& onConsumed)
    {
        size_t numAcked = acked.load();
        while (numAcked != rxData.received.load())
        {
            onConsumed(data[numAcked % N]);
            numAcked = (numAcked + 1U) % (2U * N);
            acked.store(numAcked);
        }
    }

    template<class RxData>
    bool full(RxData const& /*rxData*/) const
    {
        size_t const numSent  = sent.load();
        size_t const numAcked = acked.load();
        return (numSent == ((numAcked + N) % (2U * N)));
    }
};

template<class T, size_t N>
struct TxData<T, N, false>
{
    std::atomic<size_t> sent;
    ::etl::array<T, N> data;

    TxData() : sent(0U), data() {}

    void reset() { sent.store(0U); }

    template<class RxData>
    bool full(RxData const& rxData) const
    {
        size_t const numSent     = sent.load();
        size_t const numReceived = rxData.received.load();
        return (numSent == (numReceived + N) % (2U * N));
    }
};
} // namespace internal

} // namespace spsc
} // namespace util
