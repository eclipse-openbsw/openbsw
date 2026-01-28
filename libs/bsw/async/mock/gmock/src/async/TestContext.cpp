// Copyright 2024 Accenture.

#include "async/TestContext.h"

#include "async/AsyncMock.h"

namespace async
{
using namespace ::testing;

ACTION_P(ExecuteHelperAction, queue)
{
    RunnableType& runnable = arg1;
    if (!queue->contains(runnable))
    {
        queue->enqueue(runnable);
    }
}

ACTION_P2(ScheduleHelperAction, context, cyclic)
{
    context->schedule(arg1, arg2, static_cast<uint64_t>(arg3) * arg4, cyclic);
}

TestContext::TimeoutQueueType TestContext::_timeoutQueue;

TestContext::TestContext(ContextType context) : _context(context), _nowInUs(0U) {}

TestContext::~TestContext() { clearTimeouts(); }

void TestContext::handleExecute()
{
    EXPECT_CALL(AsyncMock::instance(), execute(_context, _))
        .WillRepeatedly(ExecuteHelperAction(&_runnableQueue));
}

void TestContext::handleSchedule()
{
    EXPECT_CALL(AsyncMock::instance(), schedule(_context, _, _, _, _))
        .WillRepeatedly(ScheduleHelperAction(this, false));
    EXPECT_CALL(AsyncMock::instance(), scheduleAtFixedRate(_context, _, _, _, _))
        .WillRepeatedly(ScheduleHelperAction(this, true));
}

void TestContext::handleAll()
{
    handleExecute();
    handleSchedule();
}

ContextType TestContext::getContext() const { return _context; }

TestContext::operator ContextType() const { return _context; }

void TestContext::execute()
{
    while (true)
    {
        RunnableValue value;
        RunnableType* runnable = nullptr;
        if (_runnableQueue.dequeue(runnable, value))
        {
            runnable->execute();
        }
        else
        {
            return;
        }
    }
}

void TestContext::schedule(
    RunnableType& runnable, TimeoutType& timeout, uint64_t delayInUs, bool cyclic)
{
    if (_timeoutQueue.contains(timeout))
    {
        return;
    }
    TimeoutValue value;
    value._context                  = this;
    value._runnable                 = &runnable;
    value._elapseTimestamp          = _nowInUs + delayInUs;
    value._period                   = cyclic ? delayInUs : 0U;
    TimeoutQueueType::Node* prev    = nullptr;
    TimeoutQueueType::Node* current = _timeoutQueue._first;
    // enqueue at end or right before the first existing element with strictly greater timestamp
    while ((current != nullptr) && (current->_value._elapseTimestamp <= value._elapseTimestamp))
    {
        prev    = current;
        current = current->_next;
    }
    _timeoutQueue.enqueue(timeout, value, prev, current);
}

void TestContext::setNow(uint64_t const nowInUs) { _nowInUs = nowInUs; }

void TestContext::elapse(uint64_t delayInUs) { _nowInUs += delayInUs; }

void TestContext::expire()
{
    TimeoutQueueType::Node* prev    = nullptr;
    TimeoutQueueType::Node* current = _timeoutQueue._first;

    while ((current != nullptr) && (current->_value._elapseTimestamp <= _nowInUs))
    {
        if (current->_value._context == this)
        {
            TimeoutType& timeout     = current->_key;
            TimeoutValue const value = current->_value;
            current                  = _timeoutQueue.remove(*current, prev);

            if (value._runnable == nullptr)
            {
                // Should not happen, but guard against it
                continue;
            }

            if (value._period > 0U)
            {
                schedule(*value._runnable, timeout, value._period, true);
            }
            value._runnable->execute();
        }
        else
        {
            prev    = current;
            current = current->_next;
        }
    }
}

void TestContext::expireAndExecute()
{
    while (_runnableQueue._first != nullptr
           || ((_timeoutQueue._first != nullptr)
               && (_timeoutQueue._first->_value._elapseTimestamp <= _nowInUs)))
    {
        expire();
        execute();
    }
}

bool TestContext::cancelTimeout(TimeoutType& timeout)
{
    TimeoutQueueType::Node* prev       = nullptr;
    TimeoutQueueType::Node* const node = _timeoutQueue.find(timeout, prev);
    if (node != nullptr)
    {
        _timeoutQueue.remove(*node, prev);
    }
    return node != nullptr;
}

void TestContext::clearTimeouts()
{
    TimeoutQueueType::Node* prev    = nullptr;
    TimeoutQueueType::Node* current = _timeoutQueue._first;
    while (current != nullptr)
    {
        if (current->_value._context == this)
        {
            current = _timeoutQueue.remove(*current, prev);
        }
        else
        {
            prev    = current;
            current = current->_next;
        }
    }
}

} // namespace async
