// Copyright 2024 Accenture.

#include <estd/forward_list.h>
#include <estd/slice.h>
#include <estd/vector.h>

#include <gmock/gmock.h>

namespace
{
// EXAMPLE_START:forward_list
/**
 * Wrapper for linked list of integers.
 */
class IntNode : public ::estd::forward_list_node<IntNode>
{
    int _i;

public:
    explicit IntNode(int const i) : _i(i) {}

    int getI() const { return _i; }
};

void fillAndPrintList()
{
    // Create two example nodes.
    IntNode a(1);
    IntNode b(2);

    // Create list of IntNodes.
    estd::forward_list<IntNode> list;
    // Add nodes to the list.
    list.push_front(a);
    list.push_front(b);

    // Iterate over list and print values.
    estd::forward_list<IntNode>::const_iterator i;
    for (i = list.cbegin(); i != list.cend(); ++i)
    {
        printf("Item = %d\n", i->getI());
    }
}

// EXAMPLE_END:forward_list

// EXAMPLE_START:forward_list_multiple_lists

/**
 * Interface for a timeout.
 */
struct ITimeout : ::estd::forward_list_node<ITimeout>
{
    /** Function to call, when isExpired() returns true */
    virtual void expired() = 0;

    /** Returns true if a timeout is expired, false otherwise */
    virtual bool isExpired() = 0;
};

/**
 * Interface for a runnable.
 */
struct IRunnable : ::estd::forward_list_node<IRunnable>
{
    virtual void run() = 0;
};

using Timeouts = ::estd::forward_list<ITimeout>;

void registerTimeout(Timeouts& timeouts, ITimeout& timeout) { timeouts.push_front(timeout); }

using Runnables = ::estd::forward_list<IRunnable>;

void registerRunnable(Runnables& runnables, IRunnable& runnable) { runnables.push_front(runnable); }

struct MyClass
: ITimeout
, IRunnable
{
    void expired() override;

    bool isExpired() override;

    void run() override;
};

void twoListExample(MyClass& worker, Timeouts& timeouts, Runnables& runnables)
{
    // Register worker to two list.
    registerTimeout(timeouts, worker);
    registerRunnable(runnables, worker);
}

// EXAMPLE_END:forward_list_multiple_lists
void MyClass::expired() {}

bool MyClass::isExpired() { return true; }

void MyClass::run() {}

struct ITimeoutMock : ITimeout
{
    MOCK_METHOD0(expired, void());
    MOCK_METHOD0(isExpired, bool());
};

// EXAMPLE_START:forward_list_move_between_lists
void callExpiredTimeouts(::estd::forward_list<ITimeout>& timeouts)
{
    // List of expired timeouts.
    ::estd::forward_list<ITimeout> expired{};
    // Maintain two iterators: one pointing to the actual element and one preceding it to be
    // used for erasing it from the list.
    auto itr    = timeouts.begin();
    auto before = timeouts.before_begin();
    // Iterate over all timeouts in list.
    while (itr != timeouts.end())
    {
        // Check if timeout is expired.
        if (itr->isExpired())
        {
            // Keep reference to expired timeout.
            ITimeout& t = *itr;
            // Erase timeout from list by calling erase_after on before iterator.
            // itr points to element after the erased one, before stays as is.
            itr         = timeouts.erase_after(before);
            // Move expired timeout to expired list
            expired.push_front(t);
        }
        else
        {
            // Adjust iterators.
            before = itr++;
        }
    }
    // Notify all expired timeouts.
    for (auto&& itr : expired)
    {
        itr.expired();
    }
}

// EXAMPLE_END:forward_list_move_between_lists

TEST(ForwardListExample, expired)
{
    using namespace ::testing;
    ITimeoutMock t1, t2, t3;
    EXPECT_CALL(t1, isExpired).WillOnce(Return(true));
    EXPECT_CALL(t2, isExpired).WillOnce(Return(false));
    EXPECT_CALL(t3, isExpired).WillOnce(Return(true));
    EXPECT_CALL(t1, expired).Times(1);
    EXPECT_CALL(t2, expired).Times(0);
    EXPECT_CALL(t3, expired).Times(1);
    ::estd::forward_list<ITimeout> timeouts;
    timeouts.push_front(t3);
    timeouts.push_front(t2);
    timeouts.push_front(t1);
    callExpiredTimeouts(timeouts);
    ASSERT_EQ(1, timeouts.size());
    ASSERT_FALSE(::estd::is_in_use(t1));
    ASSERT_TRUE(::estd::is_in_use(t2));
    ASSERT_FALSE(::estd::is_in_use(t3));
}

TEST(forward_list, run_examples)
{
    MyClass worker;
    Timeouts timeouts;
    Runnables runnables;
    twoListExample(worker, timeouts, runnables);
    fillAndPrintList();
}

namespace vector
{

// EXAMPLE_START:forward_list_vs_vector_1
struct ICanFrameListener
{
    virtual void frameReceived(uint32_t id, ::estd::slice<uint8_t> payload) = 0;
};

class CanFrameNotifier
{
    // Room for at most 10 listeners.
    ::estd::declare::vector<ICanFrameListener*, 10> _listeners;

public:
    bool addListener(ICanFrameListener& listener)
    {
        // Check if listener can be added.
        if (_listeners.full())
        {
            return false;
        }
        // Add listener to list.
        _listeners.emplace_back().construct(&listener);
        return true;
    }
};

// EXAMPLE_END:forward_list_vs_vector_1
} // namespace vector

namespace forward_list
{
// EXAMPLE_START:forward_list_vs_vector_2
struct ICanFrameListener : public ::estd::forward_list_node<ICanFrameListener>
{
    virtual void frameReceived(uint32_t id, ::estd::slice<uint8_t> payload) = 0;
};

class CanFrameNotifier
{
    // List of listeners
    ::estd::forward_list<ICanFrameListener> _listeners;

public:
    void addListener(ICanFrameListener& listener)
    {
        // Add listener to list.
        _listeners.push_front(listener);
    }
};

// EXAMPLE_END:forward_list_vs_vector_2
} // namespace forward_list

} // namespace
