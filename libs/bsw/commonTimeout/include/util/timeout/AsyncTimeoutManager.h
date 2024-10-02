// Copyright 2024 Accenture.

#ifndef GUARD_881F73AF_B733_4944_92D6_A1CBEE933F4C
#define GUARD_881F73AF_B733_4944_92D6_A1CBEE933F4C

#include "async/Types.h"
#include "util/timeout/ITimeoutManager2.h"

#include <estd/uncopyable.h>

namespace common
{
class AsyncTimeoutManager : public ITimeoutManager2
{
    UNCOPYABLE(AsyncTimeoutManager);

public:
    AsyncTimeoutManager();

    operator ::async::ContextType() const { return _context; }

    void init(::async::ContextType context);

    void init() override;

    void shutdown() override;

    ErrorCode set(AbstractTimeout& timeout, uint32_t time, bool cyclic = false) override;

    void cancel(AbstractTimeout& timeout) override;

    ::async::ContextType const& context() const { return _context; }

private:
    ::async::ContextType _context;
};

} // namespace common

#endif /*GUARD_881F73AF_B733_4944_92D6_A1CBEE933F4C*/
