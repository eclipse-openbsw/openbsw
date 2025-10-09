// Copyright 2025 BMW AG

#pragma once

#include "middleware/queue/QueueBase.h"

#include <etl/array.h>
#include <etl/error_handler.h>
#include <etl/optional.h>
#include <etl/type_traits.h>

namespace middleware
{
namespace queue
{

/**
 * \brief A queue mutex wrapper which can accept an integer, which means that the mutex would live
 * inside the queue, or a pointer, which would mean that the mutex would live outside the queue.
 * \details Each specialization will provide a init method, a get method and an alias to the wrapped
 * mutex type, which will be the same as T but with an additional volatile qualifier.
 *
 * \tparam T the mutex type which must be an integer or a pointer to an integer.
 * \tparam Specialization
 */
template<typename T, typename Specialization = void>
class QueueMutex;

/**
 * \brief Specialization for integer mutexes.
 *
 * \tparam T
 */
template<typename T>
class QueueMutex<T, typename etl::enable_if_t<etl::is_integral<T>::value>>
{
public:
    using mutex_t = etl::add_volatile_t<T>;

    /**
     * \brief Initializes the mutex's value to the one specified by \param initialValue.
     *
     * \param initialValue
     */
    void init(mutex_t initialValue = 0U) { _mutex = initialValue; }

    /**
     * \brief Get a pointer to the internal mutex variable.
     *
     * \return mutex_t*
     */
    mutex_t* get() { return &_mutex; }

private:
    mutex_t _mutex{0U};
};

/**
 * \brief Specialization for pointer to integer mutexes.
 * \details In this specialization, when creating the mutex_t alias we first remove the pointer and
 * then add the volatile to T and finally add the pointer again. This is because (volatile uint8_t*)
 * != (uint8_t* volatile) and we want the first one.
 *
 * \tparam T
 */
template<typename T>
class QueueMutex<T, typename etl::enable_if_t<etl::is_pointer<T>::value>>
{
public:
    using mutex_t = etl::add_pointer_t<etl::add_volatile_t<etl::remove_pointer_t<T>>>;

    constexpr QueueMutex()
    {
        // For now we are only accepting pointers to integral types
        static_assert(
            etl::is_pointer<T>::value
                && etl::is_integral<typename etl::remove_pointer<T>::type>::value,
            "Pointer's underlying type must be an integral");
    }

    /**
     * \brief Initializes the mutex's value to the one specified by \param initialValue.
     *
     * \param initialValue
     */
    void init(mutex_t initialValue = 0U)
    {
        ETL_ASSERT(
            initialValue != nullptr,
            "Pointer to mutex variable needs to be different than nullptr.");
        _mutex  = initialValue;
        *_mutex = 0U;
    }

    /**
     * \brief Get a pointer to the internal mutex variable.
     *
     * \return mutex_t*
     */
    mutex_t get() { return _mutex; }

private:
    mutex_t _mutex;
};

/**
 * \brief Struct encapsulating features for the queue.
 *
 * \tparam Type the object type that the queue will contain.
 * \tparam Count the number of elements of the queue.
 * \tparam Strategy the object that will be used to lock the mutex (by default is void, meaning no
 * lock mechanism should be used). \tparam TypeOfMutex the mutex type which according to QueueMutex
 * can only be an integer or a pointer to an integer.
 */
template<typename Type, uint16_t Count, typename Strategy = void, typename TypeOfMutex = uint8_t>
struct QueueTraits
{
    using T                                 = Type;
    using LockStrategy                      = Strategy;
    using MutexType                         = TypeOfMutex;
    static constexpr uint16_t ELEMENT_COUNT = Count;
};

/**
 * \brief A queue object with two specializations: one where Traits::LockStrategy is different than
 * void and another where it is void meaning that the queue doesn't need a lock mechanism. \details
 * Both specializations will provide two nested classes "Sender" and "Receiver", which are
 * constructed by receiving a reference to a queue instance, to be used for sending and receiving
 * elements.
 *
 * \tparam Traits which will be of QueueTraits type.
 */
template<typename Traits, typename Specialize = void>
class Queue;

/**
 * \brief Specialization of queue with a lock mechanism.
 *
 * \tparam Traits which will be of QueueTraits type.
 */
template<typename Traits>
class Queue<Traits, typename etl::enable_if_t<!etl::is_void<typename Traits::LockStrategy>::value>>
    final : public QueueBase
{
public:
    using Base                       = QueueBase;
    using QueueItem                  = typename Traits::T;
    using LockStrategy               = typename Traits::LockStrategy;
    using MutexType                  = QueueMutex<typename Traits::MutexType>;
    static constexpr size_t MAX_SIZE = Traits::ELEMENT_COUNT;

    /**
     * \brief Default constructor is intentionally empty, since queues will be placed in shared RAM
     * and they will be asynchronously initialized by all cores.
     *
     */
    constexpr Queue() : Base() {}

    /**
     * \brief Init method which needs to be called before doing any work with the queue.
     *
     * \param pmutex
     */
    void init(char const* const = nullptr, typename MutexType::mutex_t pmutex = 0U)
    {
        _mutex.init(pmutex);
        Base::init(MAX_SIZE);
        _buffer.fill(QueueItem{});
    }

    /**
     * \brief Nested class to read elements from the queue.
     * \details After reading an element, the advance method needs to be called in order to clear
     * the current element in the queue and get the to next element.
     *
     */
    class Receiver
    {
    public:
        explicit constexpr Receiver(Queue& queue) : _queue(queue) {}

        /**
         * \brief Gets the current number of elements that queue contains.
         *
         * \return constexpr uint32_t
         */
        constexpr uint32_t size() const { return _queue.size(); }

        /**
         * \brief Checks if the queue is empty.
         *
         * \return true if empty otherwise false.
         */
        constexpr bool isEmpty() const { return _queue.isEmpty(); }

        /**
         * \brief Gets a reference to the top element in the queue.
         *
         * \return const QueueItem&
         */
        QueueItem const& peek() const { return _queue._buffer[_queue.getReceived() % MAX_SIZE]; }

        /**
         * \brief Advance the reading cursor in the queue, thus effectively deleting the top
         * element.
         *
         */
        void advance() { _queue.advanceReceived(); }

    private:
        Queue& _queue;
    };

    /**
     * \brief Nested class to write elements to the queue.
     * \details In this specialization, this Sender class's write method uses the mutex and the
     * LockStrategy specified from QueueTraits to ensure concurrency safety.
     *
     */
    class Sender
    {
    public:
        explicit constexpr Sender(Queue& queue) : _queue(queue) {}

        /**
         * \brief Gets the current number of elements that queue contains.
         *
         * \return constexpr uint32_t
         */
        constexpr uint32_t size() const { return _queue.size(); }

        /**
         * \brief Checks if the queue is full.
         *
         * \return true if full otherwise false.
         */
        constexpr bool isFull() const { return _queue.isFull(); }

        /**
         * \brief Appends a new element to the end of the queue.
         *
         * \param value
         * \return true if the element was written successfully, otherwise false.
         */
        bool write(QueueItem const& value)
        {
            LockStrategy const lock(_queue._mutex.get());
            etl::optional<size_t> index = _queue.writeNext();
            bool res                    = index.has_value();
            if (res)
            {
                _queue._buffer[index.value()] = value;
            }

            return res;
        }

    private:
        Queue& _queue;
    };

private:
    etl::array<QueueItem, MAX_SIZE> _buffer;
    MutexType _mutex __attribute__((aligned(4)));
};

/**
 * \brief Specialization of queue without a lock mechanism, which may be useful for lock free single
 * producer single consumer queues.
 *
 * \tparam Traits which will be of QueueTraits type.
 */
template<typename Traits>
class Queue<
    Traits,
    typename etl::enable_if<etl::is_void<typename Traits::LockStrategy>::value>::type>
: public QueueBase
{
public:
    using Base                       = QueueBase;
    using QueueItem                  = typename Traits::T;
    using LockStrategy               = typename Traits::LockStrategy;
    static constexpr size_t MAX_SIZE = Traits::ELEMENT_COUNT;

    /**
     * \brief Default constructor is intentionally empty, since queues will be placed in shared RAM
     * and they will be asynchronously initialized by all cores.
     *
     */
    constexpr Queue() : Base() {}

    /**
     * \brief Init method which needs to be called before doing any work with the queue.
     *
     * \param pmutex
     */
    void init(char const* const = nullptr)
    {
        Base::init(MAX_SIZE);
        _buffer.fill(QueueItem{});
    }

    /**
     * \brief Nested class to read elements from the queue.
     * \details After reading an element, the advance method needs to be called in order to clear
     * the current element in the queue and get the to next element.
     *
     */
    class Receiver
    {
    public:
        explicit constexpr Receiver(Queue& queue) : _queue(queue) {}

        /**
         * \brief Gets the current number of elements that queue contains.
         *
         * \return constexpr uint32_t
         */
        constexpr uint32_t size() const { return _queue.size(); }

        /**
         * \brief Checks if the queue is empty.
         *
         * \return true if empty otherwise false.
         */
        constexpr bool isEmpty() const { return _queue.isEmpty(); }

        /**
         * \brief Gets a reference to the top element in the queue.
         *
         * \return const QueueItem&
         */
        QueueItem const& peek() const { return _queue._buffer[_queue.getReceived() % MAX_SIZE]; }

        /**
         * \brief Advance the reading cursor in the queue, thus effectively deleting the top
         * element.
         *
         */
        void advance() { _queue.advanceReceived(); }

    private:
        Queue& _queue;
    };

    /**
     * \brief Nested class to write elements to the queue.
     * \details In this specialization, this Sender class's write method doesn't use any mutex and
     * any LockStrategy.
     *
     */
    class Sender
    {
    public:
        explicit constexpr Sender(Queue& queue) : _queue(queue) {}

        /**
         * \brief Gets the current number of elements that queue contains.
         *
         * \return constexpr uint32_t
         */
        constexpr uint32_t size() const { return _queue.size(); }

        /**
         * \brief Checks if the queue is full.
         *
         * \return true if full otherwise false.
         */
        constexpr bool isFull() const { return _queue.isFull(); }

        /**
         * \brief Appends a new element to the end of the queue.
         *
         * \param value
         * \return true if the element was written successfully, otherwise false.
         */
        bool write(QueueItem const& value)
        {
            etl::optional<size_t> index = _queue.writeNext();
            bool const res              = index.has_value();
            if (res)
            {
                _queue._buffer[index.value()] = value;
            }

            return res;
        }

    private:
        Queue& _queue;
    };

private:
    etl::array<QueueItem, MAX_SIZE> _buffer;
};

static_assert(
    (sizeof(Queue<QueueTraits<etl::array<uint8_t, 32U>, 5U>>) % sizeof(uint32_t)) == 0U,
    "Performance penalty due to misaligned queue!");

} // namespace queue
} // namespace middleware
