// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_494723A2_B0E1_4BA7_B959_49EFF53DA1FB
#define GUARD_494723A2_B0E1_4BA7_B959_49EFF53DA1FB

#include "async/Async.h"

#include <estd/functional.h>

namespace async
{
template<class T>
class Call : public RunnableType
{
public:
    using CallType = T;
    using RunnableType::RunnableType;

    Call(T const& call);

    void execute() override;

private:
    T _call;
};

using Function = Call<::estd::function<void()>>;

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

#endif // _ASYNC_ASYNCCALL_H_
