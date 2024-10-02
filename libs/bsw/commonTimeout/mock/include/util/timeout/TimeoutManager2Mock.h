// Copyright 2024 Accenture.

#ifndef GUARD_33115040_6944_4ABB_BABB_D5B5C3E79538
#define GUARD_33115040_6944_4ABB_BABB_D5B5C3E79538

#include "util/timeout/AbstractTimeout.h"
#include "util/timeout/ITimeoutManager2.h"

#include <gmock/gmock.h>

namespace common
{
class TimeoutManager2Mock : public ITimeoutManager2
{
public:
    virtual ~TimeoutManager2Mock()
    {
        while (_first != nullptr)
        {
            TimerInfo* remove = _first;
            _first            = _first->_next;
            delete remove;
        }
    }

    TimeoutManager2Mock() : _first(nullptr)
    {
        using namespace ::testing;
        ON_CALL(*this, set(_, _, _))
            .WillByDefault(Invoke(this, &TimeoutManager2Mock::setImplementation));
        ON_CALL(*this, cancel(_))
            .WillByDefault(Invoke(this, &TimeoutManager2Mock::cancelImplementation));
    }

    MOCK_METHOD0(init, void());
    MOCK_METHOD0(shutdown, void());
    MOCK_METHOD3(set, ErrorCode(AbstractTimeout&, uint32_t, bool));
    MOCK_METHOD1(cancel, void(AbstractTimeout&));

    void expire(AbstractTimeout& timeout)
    {
        if (timeout._isActive)
        {
            timeout._isActive = timeout._isCyclic;
            timeout.expired(AbstractTimeout::TimeoutExpiredActions(timeout));
        }
    }

private:
    struct TimerInfo
    {
        TimerInfo(AbstractTimeout& timeout, TimerInfo* next) : _timeout(&timeout), _next(next) {}

        AbstractTimeout* _timeout;
        TimerInfo* _next;
    };

    ErrorCode setImplementation(AbstractTimeout& timeout, uint32_t time, bool isCyclic)
    {
        getTimerInfo(timeout);
        timeout._time     = time;
        timeout._isCyclic = isCyclic;
        timeout._isActive = true;
        return ITimeoutManager2::TIMEOUT_OK;
    }

    void cancelImplementation(AbstractTimeout& timeout)
    {
        getTimerInfo(timeout)._timeout->_isActive = false;
    }

    TimerInfo& getTimerInfo(AbstractTimeout& timeout)
    {
        TimerInfo* info = _first;
        while ((info != nullptr) && (info->_timeout != &timeout))
        {
            info = info->_next;
        }
        if (info == nullptr)
        {
            info   = new TimerInfo(timeout, _first);
            _first = info;
        }
        return *info;
    }

    TimerInfo* _first;
};

} // namespace common

#endif /* GUARD_33115040_6944_4ABB_BABB_D5B5C3E79538 */
