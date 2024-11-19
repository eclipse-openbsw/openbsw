// Copyright 2024 Accenture.

#ifndef GUARD_11075096_3ACA_4F81_92A2_406CEF298C94
#define GUARD_11075096_3ACA_4F81_92A2_406CEF298C94

#include "async/Types.h"

namespace async
{
/**
 * A template class that lets specify member function
 * of eventually non-runnable class to be called as result of
 * async::execute(), async::schedule() or async::scheduleAtFixedRate(),
 * this way mimicking non-runnable class to act as runnable.
 *
 * \tparam Handler The type of (non-runnable) class.
 * \tparam handleFunc The member function to be called.
 */
template<typename Handler, void (Handler::*handleFunc)()>
class MemberCall : public RunnableType
{
public:
    /**
     * Constructor.
     * \param handler Reference to the handler object.
     */
    MemberCall(Handler& handler) : _handler(handler) {}

private:
    void execute() override { (_handler.*handleFunc)(); }

private:
    Handler& _handler;
};
} // namespace async

#endif
