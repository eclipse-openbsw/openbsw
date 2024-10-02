// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_ECCAA06F_729C_4CDD_9C3B_67D97352DE9E
#define GUARD_ECCAA06F_729C_4CDD_9C3B_67D97352DE9E

#include "async/Queue.h"

#include <platform/config.h>

namespace async
{
template<typename Runnable, typename EventPolicy, typename Lock>
class RunnableExecutor
{
public:
    explicit RunnableExecutor(typename EventPolicy::EventDispatcherType& eventDispatcher);

    void init();
    void shutdown();

    void enqueue(Runnable& runnable);

private:
    void handleEvent();

    Queue<Runnable> _queue;
    EventPolicy _eventPolicy;
};

/**
 * Inline implementations.
 */
template<typename Runnable, typename EventPolicy, typename Lock>
RunnableExecutor<Runnable, EventPolicy, Lock>::RunnableExecutor(
    typename EventPolicy::EventDispatcherType& eventDispatcher)
: _queue(), _eventPolicy(eventDispatcher)
{}

template<typename Runnable, typename EventPolicy, typename Lock>
void RunnableExecutor<Runnable, EventPolicy, Lock>::init()
{
    _eventPolicy.setEventHandler(
        EventPolicy::HandlerFunctionType::
            template create<RunnableExecutor, &RunnableExecutor::handleEvent>(*this));
}

template<typename Runnable, typename EventPolicy, typename Lock>
void RunnableExecutor<Runnable, EventPolicy, Lock>::shutdown()
{
    _eventPolicy.removeEventHandler();
}

template<typename Runnable, typename EventPolicy, typename Lock>
inline void RunnableExecutor<Runnable, EventPolicy, Lock>::enqueue(Runnable& runnable)
{
    {
        ESR_UNUSED const Lock lock;
        if (!runnable.isEnqueued())
        {
            _queue.enqueue(runnable);
        }
    }
    _eventPolicy.setEvent();
}

template<typename Runnable, typename EventPolicy, typename Lock>
void RunnableExecutor<Runnable, EventPolicy, Lock>::handleEvent()
{
    while (true)
    {
        Runnable* runnable;
        {
            ESR_UNUSED const Lock lock;
            runnable = _queue.dequeue();
        }
        if (runnable != nullptr)
        {
            runnable->execute();
        }
        else
        {
            break;
        }
    }
}

} // namespace async

#endif // GUARD_ECCAA06F_729C_4CDD_9C3B_67D97352DE9E
