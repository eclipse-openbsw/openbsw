// Copyright 2024 Accenture.

#ifndef GUARD_229D2FB3_F055_4DC5_99DB_D6E35310F81B
#define GUARD_229D2FB3_F055_4DC5_99DB_D6E35310F81B

#include "async/Types.h"

#include <estd/uncopyable.h>

#include <cstdint>

namespace common
{
/**
 * Base class for timeout based action classes
 * \note this base class can be used for software based timers
 */
class AbstractTimeout : public ::async::RunnableType
{
    UNCOPYABLE(AbstractTimeout);

public:
    class TimeoutExpiredActions
    {
    public:
        explicit TimeoutExpiredActions(AbstractTimeout& timeout);

        TimeoutExpiredActions(TimeoutExpiredActions const& rhs);

        void reset();
        void reset(uint32_t newTimeout);
        void cancel();

    private:
        AbstractTimeout& _timeout;
    };

    AbstractTimeout() = default;

    /**
     * Callback that gets called once a set timeout expires.
     * \param actions TimeoutExpiredActions that can be used to reset or
     * cancel the current timeout.
     * The parameter actions is passed by value on purpose!
     * \note
     * This callback is called from the ITimeoutManager2s task context
     */
    virtual void expired(TimeoutExpiredActions actions) = 0;

    void execute() override;

    bool isActive() const;

    ::async::TimeoutType _asyncTimeout{};
    uint32_t _time                = 0U;
    ::async::ContextType _context = ::async::CONTEXT_INVALID;
    bool _isCyclic                = false;
    bool _isActive                = false;
};

inline bool operator==(AbstractTimeout const& lhs, AbstractTimeout const& rhs)
{
    return &lhs == &rhs;
}

} // namespace common

#endif /*GUARD_229D2FB3_F055_4DC5_99DB_D6E35310F81B*/
