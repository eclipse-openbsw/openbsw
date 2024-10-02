// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_ECBA8222_466A_46D4_AA94_CC1FF5DCC677
#define GUARD_ECBA8222_466A_46D4_AA94_CC1FF5DCC677

#include "async/Async.h"

#include <estd/forward_list.h>

namespace async
{
class TestContext
{
public:
    explicit TestContext(ContextType context);
    ~TestContext();

    void handleExecute();
    void handleSchedule();
    void handleAll();

    ContextType getContext() const;
    operator ContextType() const;

    void execute(RunnableType& runnable);

    void schedule(RunnableType& runnable, TimeoutType& timeout, uint64_t delayInUs, bool cyclic);

    /// Execute all runnables queued via the `async::execute` function, including all new runnables
    /// queued during this process.
    void execute();
    /// Execute all expired runnables queued via the `async::schedule` functions.
    void expire();
    /// Execute all expired and queued runnables and ensure that any new runnables queued or
    /// scheduled during this process are also executed.
    void expireAndExecute();

    /// Set absolute time used by `expire` to determine which scheduled runnables to execute.
    void setNow(uint64_t nowInUs);
    /// Increment time used by `expire` to determine which scheduled runnables to execute.
    void elapse(uint64_t delayInUs);

private:
    friend class TimeoutType;
    static bool cancelTimeout(TimeoutType& timeout);

    void clearTimeouts();

    struct RunnableValue
    {};

    struct TimeoutValue
    {
        TimeoutValue() = default;

        TestContext* _context     = nullptr;
        RunnableType* _runnable   = nullptr;
        uint64_t _elapseTimestamp = 0U;
        uint32_t _period          = 0U;
    };

    template<class Key, class Value>
    struct Queue
    {
    public:
        struct Node
        {
            Node(Key& key, Value const& value) : _key(key), _value(value) {}

            Key& _key;
            Value _value;
            Node* _next = nullptr;
        };

        Queue() = default;

        ~Queue() { clear(); }

        bool contains(Key& key) const
        {
            Node* prev;
            return find(key, prev) != nullptr;
        }

        Node* find(Key& key, Node*& prev) const
        {
            prev = nullptr;
            for (Node* node = _first; node != nullptr; node = node->_next)
            {
                if (&node->_key == &key)
                {
                    return node;
                }
                prev = node;
            }
            return nullptr;
        }

        void enqueue(Key& key) { enqueue(key, Value(), _last, nullptr); }

        void enqueue(Key& key, Value const& value, Node* prev, Node* next)
        {
            Node* node = new Node(key, value);
            if (prev == nullptr)
            {
                _first = node;
            }
            else
            {
                prev->_next = node;
            }
            node->_next = next;
            if (next == nullptr)
            {
                _last = node;
            }
        }

        bool dequeue(Key*& key, Value& value)
        {
            Node* node = _first;
            if (node != nullptr)
            {
                key    = &node->_key;
                value  = node->_value;
                _first = node->_next;
                if (_first == nullptr)
                {
                    _last = nullptr;
                }
                delete node;
                return true;
            }
            return false;
        }

        Node* remove(Node& node, Node* prev)
        {
            Node* next = node._next;
            if (prev == nullptr)
            {
                _first = node._next;
            }
            else
            {
                prev->_next = node._next;
            }
            if (node._next == nullptr)
            {
                _last = prev;
            }
            delete &node;
            return next;
        }

        void clear()
        {
            while (_first != nullptr)
            {
                Node* node = _first;
                _first     = node->_next;
                delete node;
            }
            _last = nullptr;
        }

        Node* _first = nullptr;
        Node* _last  = nullptr;
    };

    using TimeoutQueueType = Queue<TimeoutType, TimeoutValue>;

    Queue<RunnableType, RunnableValue> _runnableQueue;
    ContextType _context;
    uint64_t _nowInUs;
    static TimeoutQueueType _timeoutQueue;
};

} // namespace async

#endif // GUARD_ECBA8222_466A_46D4_AA94_CC1FF5DCC677
