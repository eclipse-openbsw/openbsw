// Copyright 2024 Accenture.

#ifndef GUARD_11075096_3ACA_4F81_92A2_406CEF298C94
#define GUARD_11075096_3ACA_4F81_92A2_406CEF298C94

#include "async/Types.h"

namespace async
{
template<typename Handler, void (Handler::*handleFunc)()>
class MemberCall : public RunnableType
{
public:
    MemberCall(Handler& handler) : _handler(handler) {}

    void execute() override { (_handler.*handleFunc)(); }

private:
    Handler& _handler;
};
} // namespace async

#endif
