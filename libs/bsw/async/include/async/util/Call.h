// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"

#include <etl/delegate.h>

namespace async
{
/**
 * A template class that allows to provide a specific
 * function to be executed as result of
 * async::execute(), async::schedule() or async::scheduleAtFixedRate(),
 *
 * \tparam Handler The type of (non-runnable) class.
 * \tparam handleFunc The member function to be called.
 */
template<class T>
class Call : public RunnableType
{
public:
    using CallType = T;
    using RunnableType::RunnableType;

    /**
     * Constructor.
     * \param call function to execute provided by client.
     */
    Call(T const& call);

private:
    void execute() override;

private:
    T _call;
};

using Function = Call<::etl::delegate<void()>>;

/**
 * Inline implementations.
 */
template<class T>
Call<T>::Call(T const& call) : _call(call)
{}

template<class T>
void Call<T>::execute()
{
    _call();
}

} // namespace async
